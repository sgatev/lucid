#include "lucid/arm64/macho.h"

#include <mach-o/loader.h>
#include <mach-o/nlist.h>
#include <mach-o/reloc.h>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "lucid/arm64/assembler.h"
#include "lucid/core/testing/testing.h"

namespace lucid {
namespace {

// The object that `assembler` compiles to, read back with the definitions the
// system linker reads it with.
//
// Reading it back that way is the point: a field this writer fills in wrongly
// is a field every tool that opens the object disagrees about.
class Object {
 public:
  explicit Object(const arm64::Assembler& assembler) {
    std::ostringstream out;
    WriteCompiledMachObject(assembler, out);
    bytes_ = out.str();
  }

  const std::string& Bytes() const { return bytes_; }

  mach_header_64 Header() const { return Read<mach_header_64>(0); }

  // Returns the load commands, walked the way a reader walks them: from the
  // count and the sizes the header gives.
  std::vector<load_command> LoadCommands() const {
    std::vector<load_command> commands;
    commands.reserve(Header().ncmds);

    std::size_t offset = sizeof(mach_header_64);
    for (std::uint32_t i = 0; i < Header().ncmds; ++i) {
      commands.push_back(Read<load_command>(offset));
      offset += commands.back().cmdsize;
    }
    return commands;
  }

  // Returns the first load command of the given `type`, if a reader reaches
  // one.
  template <typename T>
  std::optional<T> Command(std::uint32_t type) const {
    const std::optional<std::size_t> offset = CommandOffset(type);
    if (!offset.has_value()) return std::nullopt;
    return Read<T>(*offset);
  }

  // Returns the only section of the only segment.
  section_64 Section() const {
    return Read<section_64>(*CommandOffset(LC_SEGMENT_64) +
                            sizeof(segment_command_64));
  }

  // Returns the symbols in the symbol table.
  std::vector<nlist_64> Symbols() const {
    const symtab_command symtab = *Command<symtab_command>(LC_SYMTAB);

    std::vector<nlist_64> symbols;
    symbols.reserve(symtab.nsyms);
    for (std::uint32_t i = 0; i < symtab.nsyms; ++i) {
      symbols.push_back(Read<nlist_64>(symtab.symoff + i * sizeof(nlist_64)));
    }
    return symbols;
  }

  // Returns the name `symbol` carries in the string table.
  std::string_view SymbolName(const nlist_64& symbol) const {
    const symtab_command symtab = *Command<symtab_command>(LC_SYMTAB);
    return bytes_.data() + symtab.stroff + symbol.n_un.n_strx;
  }

  // Returns the relocations of the only section.
  std::vector<relocation_info> Relocations() const {
    const section_64 section = Section();

    std::vector<relocation_info> relocations;
    relocations.reserve(section.nreloc);
    for (std::uint32_t i = 0; i < section.nreloc; ++i) {
      relocations.push_back(
          Read<relocation_info>(section.reloff + i * sizeof(relocation_info)));
    }
    return relocations;
  }

 private:
  // Returns where the first load command of the given `type` begins.
  std::optional<std::size_t> CommandOffset(std::uint32_t type) const {
    std::size_t offset = sizeof(mach_header_64);
    for (std::uint32_t i = 0; i < Header().ncmds; ++i) {
      const load_command command = Read<load_command>(offset);
      if (command.cmd == type) return offset;
      offset += command.cmdsize;
    }
    return std::nullopt;
  }

  // Returns a copy of the `T` at `offset`.
  //
  // A copy, because the object is a string of bytes and promises nothing about
  // where a structure within it lands: reading one in place would be reading a
  // misaligned one.
  template <typename T>
  T Read(std::size_t offset) const {
    T value;
    std::memcpy(&value, bytes_.data() + offset, sizeof(T));
    return value;
  }

  std::string bytes_;
};

TEST(Test, HeaderDescribesAnArm64Object) {
  arm64::Assembler assembler;
  assembler.Ret();

  const Object object(assembler);

  EXPECT_EQ(object.Header().magic, MH_MAGIC_64);
  EXPECT_EQ(object.Header().cputype, CPU_TYPE_ARM64);
  EXPECT_EQ(object.Header().cpusubtype, CPU_SUBTYPE_ARM64_ALL);
  EXPECT_EQ(object.Header().filetype, MH_OBJECT);
}

TEST(Test, HeaderCountsEveryLoadCommandItWrites) {
  arm64::Assembler assembler;
  assembler.Ret();

  const Object object(assembler);

  // A count short of the commands that follow leaves the rest of them unread,
  // and `sizeofcmds` then measures commands the count does not admit to.
  std::vector<std::uint32_t> command_types;
  std::uint32_t commands_size = 0;
  for (const load_command& command : object.LoadCommands()) {
    command_types.push_back(command.cmd);
    commands_size += command.cmdsize;
  }

  EXPECT_THAT(command_types, ElementsEqual(std::uint32_t{LC_SEGMENT_64},
                                           std::uint32_t{LC_BUILD_VERSION},
                                           std::uint32_t{LC_SYMTAB},
                                           std::uint32_t{LC_DYSYMTAB}));
  EXPECT_EQ(object.Header().sizeofcmds, commands_size);
}

TEST(Test, SegmentHoldsTheAssembledInstructions) {
  arm64::Assembler assembler;
  assembler.Ret();
  assembler.Ret();

  const Object object(assembler);
  const section_64 section = object.Section();

  EXPECT_EQ(std::string_view(section.sectname, 6), "__text");
  EXPECT_EQ(std::string_view(section.segname, 6), "__TEXT");
  EXPECT_EQ(section.size, assembler.OutputBytesCount());

  std::ostringstream instructions;
  assembler.WriteBytes(instructions);
  EXPECT_EQ(object.Bytes().substr(section.offset, section.size),
            instructions.str());
}

TEST(Test, GlobalLabelsBecomeDefinedSymbols) {
  arm64::Assembler assembler;
  assembler.Global("_main");
  assembler.Ret();

  const Object object(assembler);
  const std::vector<nlist_64> symbols = object.Symbols();

  ASSERT_EQ(symbols.size(), 1u);
  EXPECT_EQ(object.SymbolName(symbols.front()), "_main");
  EXPECT_EQ(symbols.front().n_type, N_SECT | N_EXT);
  EXPECT_EQ(symbols.front().n_sect, 1);
}

TEST(Test, ExternalLabelsBecomeUndefinedSymbolsAndRelocations) {
  arm64::Assembler assembler;
  assembler.Bl(assembler.External("_printf"));

  const Object object(assembler);
  const std::vector<nlist_64> symbols = object.Symbols();

  ASSERT_EQ(symbols.size(), 1u);
  EXPECT_EQ(object.SymbolName(symbols.front()), "_printf");
  EXPECT_EQ(symbols.front().n_type, N_UNDF | N_EXT);
  EXPECT_EQ(symbols.front().n_sect, NO_SECT);

  // The call has to be pointed at the symbol once the linker knows where it
  // landed, which is what the relocation is for.
  const std::vector<relocation_info> relocations = object.Relocations();
  ASSERT_EQ(relocations.size(), 1u);
  EXPECT_EQ(relocations.front().r_address, 0);
  EXPECT_EQ(relocations.front().r_extern, 1u);
  EXPECT_EQ(relocations.front().r_pcrel, 1u);
}

TEST(Test, AnObjectWithoutCallsHasNoRelocations) {
  arm64::Assembler assembler;
  assembler.Ret();

  const Object object(assembler);

  // Nothing here refers to a symbol, so claiming a relocation would point the
  // linker at bytes that are not one.
  EXPECT_EQ(object.Section().nreloc, 0u);
  EXPECT_EQ(object.Symbols().size(), 0u);
}

TEST(Test, DefinedAndUndefinedSymbolsAreIndexedForTheLinker) {
  arm64::Assembler assembler;
  assembler.Global("_main");
  assembler.Bl(assembler.External("_printf"));

  const Object object(assembler);
  const std::optional<dysymtab_command> dysymtab =
      object.Command<dysymtab_command>(LC_DYSYMTAB);
  // A command the header does not count is a command no reader reaches.
  ASSERT_TRUE(dysymtab.has_value());

  // The defined symbols come first and the undefined ones after them, and the
  // dynamic symbol table says where each run starts and how long it is.
  const std::vector<nlist_64> symbols = object.Symbols();
  ASSERT_EQ(symbols.size(), 2u);
  EXPECT_EQ(object.SymbolName(symbols[0]), "_main");
  EXPECT_EQ(object.SymbolName(symbols[1]), "_printf");
  EXPECT_EQ(dysymtab->iextdefsym, 0u);
  EXPECT_EQ(dysymtab->nextdefsym, 1u);
  EXPECT_EQ(dysymtab->iundefsym, 1u);
  EXPECT_EQ(dysymtab->nundefsym, 1u);
}

}  // namespace
}  // namespace lucid
