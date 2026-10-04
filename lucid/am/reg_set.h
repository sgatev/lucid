#pragma once

#include <algorithm>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <limits>
#include <vector>

#include "lucid/am/instructions.h"

namespace lucid {

// A dense set of registers.
class RegSet {
 public:
  explicit RegSet(std::size_t reg_count) : indices_(reg_count, kAbsent) {}

  // Returns whether `reg` is in the set.
  bool Contains(Reg reg) const { return indices_[reg.id] != kAbsent; }

  // Inserts `reg` in the set.
  void Insert(Reg reg) {
    if (Contains(reg)) return;

    indices_[reg.id] = static_cast<std::uint32_t>(regs_.size());
    regs_.push_back(reg);
  }

  // Removes `reg` from the set.
  void Remove(Reg reg) {
    const std::uint32_t reg_idx = indices_[reg.id];
    if (reg_idx == kAbsent) return;

    regs_[reg_idx] = regs_.back();
    indices_[regs_[reg_idx].id] = reg_idx;
    regs_.pop_back();
    indices_[reg.id] = kAbsent;
  }

  // Empties the set.
  void Clear() {
    for (Reg reg : regs_) indices_[reg.id] = kAbsent;
    regs_.clear();
  }

  // Returns the number of unique registers inserted so far.
  std::size_t size() const { return regs_.size(); }

  // Returns an iterator referring to the first register in the set or `end()`,
  // if there isn't one.
  auto begin() const { return regs_.begin(); }

  // Returns an iterator past the last register in the set.
  auto end() const { return regs_.end(); }

 private:
  static constexpr std::uint32_t kAbsent =
      std::numeric_limits<std::uint32_t>::max();

  // Dense set of registers.
  std::vector<Reg> regs_;

  // Index of a register (by register id) in `regs_`.
  std::vector<std::uint32_t> indices_;
};

// A set of registers held as a bit for each register ID, with a bit beside it
// for whether the register is 64 bits wide, which is what it takes to give
// back the register an ID stands for.
//
// The bits are grouped in words of 64 IDs, and only the words that hold a
// register are kept, in order of the IDs they cover. A set costs as much as
// the registers it holds, rather than as much as the highest of them: a long
// function keeps a few early values live while its IDs climb, and a word for
// every ID below the highest, in every block, is quadratic in its length.
//
// Putting in every register of another set and comparing two sets are passes
// over the words of the two, and those are what a dataflow analysis does most.
class RegBitSet {
 private:
  // The registers of 64 consecutive IDs, starting from `index` times 64.
  struct Word {
    bool operator==(const Word&) const = default;

    std::uint32_t index;
    // Set for the registers held. Never zero for a word that is kept.
    std::uint64_t present;
    // Set for the registers held that are 64 bits wide.
    std::uint64_t wide;
  };

 public:
  // Walks the registers of a set, by ascending ID.
  class Iterator {
   public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = Reg;
    using difference_type = std::ptrdiff_t;
    using pointer = void;
    using reference = Reg;

    Iterator() = default;

    Reg operator*() const {
      const Word& word = (*words_)[word_];
      const int bit = std::countr_zero(bits_);
      const bool wide = (word.wide & (std::uint64_t{1} << bit)) != 0;
      return Reg{
          .id = static_cast<std::int32_t>(word.index * kWordBits + bit),
          .size = wide ? RegSize64 : RegSize32,
      };
    }

    Iterator& operator++() {
      bits_ &= bits_ - 1;
      if (bits_ == 0 && ++word_ < words_->size()) {
        bits_ = (*words_)[word_].present;
      }
      return *this;
    }

    Iterator operator++(int) {
      Iterator before = *this;
      ++*this;
      return before;
    }

    bool operator==(const Iterator& other) const {
      return word_ == other.word_ && bits_ == other.bits_;
    }

   private:
    friend class RegBitSet;

    Iterator(const std::vector<Word>& words, std::size_t word)
        : words_(&words),
          word_(word),
          bits_(word < words.size() ? words[word].present : 0) {}

    const std::vector<Word>* words_ = nullptr;
    std::size_t word_ = 0;
    // The registers of the current word not yet walked.
    std::uint64_t bits_ = 0;
  };

  // A word is kept only while it holds a register, so two sets holding the
  // same registers keep the same words.
  bool operator==(const RegBitSet& other) const = default;

  // Returns whether `reg` is in the set.
  bool Contains(Reg reg) const {
    const auto it = Find(IndexOf(reg));
    return it != words_.end() && it->index == IndexOf(reg) &&
           (it->present & BitOf(reg)) != 0;
  }

  // Inserts `reg` in the set.
  void Insert(Reg reg) {
    auto it = Find(IndexOf(reg));
    if (it == words_.end() || it->index != IndexOf(reg)) {
      it = words_.insert(it,
                         Word{.index = IndexOf(reg), .present = 0, .wide = 0});
    }
    it->present |= BitOf(reg);
    if (reg.size == RegSize64) {
      it->wide |= BitOf(reg);
    } else {
      it->wide &= ~BitOf(reg);
    }
  }

  // Removes `reg` from the set.
  void Remove(Reg reg) {
    const auto it = Find(IndexOf(reg));
    if (it == words_.end() || it->index != IndexOf(reg)) return;

    it->present &= ~BitOf(reg);
    it->wide &= ~BitOf(reg);
    if (it->present == 0) words_.erase(it);
  }

  // Inserts every register in `other`.
  void InsertAll(const RegBitSet& other) {
    if (words_.empty()) {
      words_ = other.words_;
      return;
    }

    // How many words the two have between them, so that they can be merged
    // in place, from the back, into room made for all of them at once.
    std::size_t merged = words_.size() + other.words_.size();
    for (std::size_t i = 0, j = 0;
         i < words_.size() && j < other.words_.size();) {
      if (words_[i].index < other.words_[j].index) {
        ++i;
      } else if (other.words_[j].index < words_[i].index) {
        ++j;
      } else {
        --merged;
        ++i;
        ++j;
      }
    }

    std::size_t i = words_.size();
    std::size_t j = other.words_.size();
    std::size_t k = merged;
    words_.resize(merged);
    // Once `other` has run out, what is left of this set is already where it
    // belongs.
    while (j > 0) {
      const Word& theirs = other.words_[j - 1];
      if (i > 0 && words_[i - 1].index > theirs.index) {
        words_[--k] = words_[--i];
      } else if (i > 0 && words_[i - 1].index == theirs.index) {
        Word word = words_[--i];
        word.present |= theirs.present;
        word.wide |= theirs.wide;
        words_[--k] = word;
        --j;
      } else {
        words_[--k] = theirs;
        --j;
      }
    }
  }

  // Returns the number of registers in the set.
  std::size_t size() const {
    std::size_t count = 0;
    for (const Word& word : words_) count += std::popcount(word.present);
    return count;
  }

  // Returns whether the set holds no register.
  bool empty() const { return words_.empty(); }

  Iterator begin() const { return Iterator(words_, 0); }

  Iterator end() const { return Iterator(words_, words_.size()); }

 private:
  static constexpr std::size_t kWordBits = 64;

  static std::uint32_t IndexOf(Reg reg) {
    assert(reg.id >= 0);
    return static_cast<std::uint32_t>(reg.id) / kWordBits;
  }

  static std::uint64_t BitOf(Reg reg) {
    return std::uint64_t{1} << (static_cast<std::uint32_t>(reg.id) % kWordBits);
  }

  // Returns the first word that covers IDs from `index` times 64 on.
  std::vector<Word>::iterator Find(std::uint32_t index) {
    return std::ranges::lower_bound(words_, index, {}, &Word::index);
  }

  std::vector<Word>::const_iterator Find(std::uint32_t index) const {
    return std::ranges::lower_bound(words_, index, {}, &Word::index);
  }

  // The words that hold a register, by ascending index.
  std::vector<Word> words_;
};

}  // namespace lucid
