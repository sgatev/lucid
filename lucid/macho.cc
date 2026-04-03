#include "lucid/macho.h"

#include <cstdint>
#include <ostream>
#include <vector>

#include "lucid/arm64/assembler.h"

namespace lucid {
namespace {

// Capability bits used in the definition of `CpuType`.
enum CpuArch : int {
  Abi64 = 0x01000000,
};

// Machine types.
enum class CpuType : int {
  Arm = 12,
  Arm64 = Arm | CpuArch::Abi64,
};

// Machine subtypes.
enum class CpuSubType : int {
  Arm64All = 0,
};

// VM protection values.
enum class VmProt : int {
  Read = 0x01,
  Write = 0x02,
  Execute = 0x04,
  ReadWriteExecute = Read | Write | Execute,
};

// Values for the `file_type` field of the `MachHeader64`.
enum class FileType : std::uint32_t {
  // Relocatable object file.
  Object = 0x1,
};

// Constants for the section attributes part of the flags field of a section
// structure.
// Section contains only true machine instructions.
constexpr std::uint32_t kAttrPureInstructions = 0x80000000;

// Section contains some machine instructions.
constexpr std::uint32_t kAttrSomeInstructions = 0x00000400;

// A 64-bit mach header that appears at the beginning of object files for 64-bit
// architectures.
struct MachHeader64 {
  struct {
    std::uint32_t magic = 0xfeedfacf;
  };
  CpuType cpu_type;
  CpuSubType cpu_sub_type;
  FileType file_type;
  std::uint32_t ncmds;
  std::uint32_t sizeofcmds;
  std::uint32_t flags;
  std::uint32_t reserved;
};

struct SegmentCommand64 {
  struct {
    std::uint32_t cmd = 0x19;
  };
  std::uint32_t cmdsize;
  char segname[16];
  std::uint64_t vmaddr;
  std::uint64_t vmsize;
  std::uint64_t fileoff;
  std::uint64_t filesize;
  VmProt maxprot;
  VmProt initprot;
  std::uint32_t nsects;
  std::uint32_t flags;
};

// Command that contains the min OS version on which this binary was built to
// run for its platform. The list of known platforms and tool values following
// it.
struct BuildVersionCommand {
  struct {
    // Fixed identifier of the command.
    std::uint32_t cmd = 0x32;
  };

  // Size of the command.
  //
  // Should be set to `sizeof(BuildVersionCommand) + ntools *
  // sizeof(BuildToolVersion)`.
  std::uint32_t cmdsize;

  // Platform.
  std::uint32_t platform;

  // X.Y.Z is encoded in nibbles xxxx.yy.zz.
  std::uint32_t minos;

  // X.Y.Z is encoded in nibbles xxxx.yy.zz.
  std::uint32_t sdk;

  // Number of tool entries following this command.
  std::uint32_t ntools;
};

// Command that contains the offsets and sizes of the link-edit 4.3BSD "stab"
// style symbol table information.
struct SymTabCommand {
  struct {
    // Fixed identifier of the command.
    std::uint32_t cmd = 0x2;

    // Fixed size of the command.
    std::uint32_t cmdsize = sizeof(SymTabCommand);
  };

  // Symbol table offset.
  std::uint32_t symoff;

  // Number of symbol table entries.
  std::uint32_t nsyms;

  // String table offset.
  std::uint32_t stroff;

  // String table size in bytes.
  std::uint32_t strsize;
};

struct DySymTabCommand {
  struct {
    std::uint32_t cmd = 0xb;
    std::uint32_t cmdsize = sizeof(DySymTabCommand);
  };

  std::uint32_t ilocalsym;
  std::uint32_t nlocalsym;
  std::uint32_t iextdefsym;
  std::uint32_t nextdefsym;
  std::uint32_t iundefsym;
  std::uint32_t nundefsym;
  std::uint32_t tocoff;
  std::uint32_t ntoc;
  std::uint32_t modtaboff;
  std::uint32_t nmodtab;
  std::uint32_t extrefsymoff;
  std::uint32_t nextrefsyms;
  std::uint32_t indirectsymoff;
  std::uint32_t nindirectsyms;
  std::uint32_t extreloff;
  std::uint32_t nextrel;
  std::uint32_t locreloff;
  std::uint32_t nlocrel;
};

struct Section64 {
  char sectname[16];
  char segname[16];
  std::uint64_t addr;
  std::uint64_t size;
  std::uint32_t offset;
  std::uint32_t align;
  std::uint32_t reloff;
  std::uint32_t nreloc;
  std::uint32_t flags;
  std::uint32_t reserved1;
  std::uint32_t reserved2;
  std::uint32_t reserved3;
};

struct NList64 {
  union {
    std::uint32_t n_strx; /* index into the string table */
  } n_un;
  std::uint8_t n_type;   /* type flag, see below */
  std::uint8_t n_sect;   /* section number or NO_SECT */
  std::uint16_t n_desc;  /* see <mach-o/stab.h> */
  std::uint64_t n_value; /* value of this symbol (or stab offset) */
};

struct RelocationInfo {
  int32_t r_address;
  uint32_t r_symbolnum : 24, r_pcrel : 1, r_length : 2, r_extern : 1,
      r_type : 4;
};

template <typename T>
void WriteStruct(const T& obj, std::ostream& out) {
  out.write(reinterpret_cast<const char*>(&obj), sizeof(T));
}

}  // namespace

void WriteCompiledMachObject(const arm64::Assembler& assembler,
                             std::ostream& out) {
  std::uint32_t sym_num = 0;
  std::uint32_t sym_str_size = 1;

  std::vector<NList64> ext_def_syms;
  for (const auto& [label, _] : assembler.GlobalLabels()) {
    ext_def_syms.push_back({
        .n_un{
            .n_strx = sym_str_size,
        },
        .n_type = 0xe /*N_SECT*/ | 0x01 /*N_EXT*/,
        .n_sect = 1,
        .n_desc = 0,
        .n_value = 0,
    });
    ++sym_num;
    sym_str_size += label.size() + 1;
  }

  std::vector<NList64> undef_syms;
  std::vector<RelocationInfo> relocs;
  for (const auto& [label, positions] : assembler.ExternalLabels()) {
    undef_syms.push_back({
        .n_un{
            .n_strx = sym_str_size,
        },
        .n_type = 0x0 /*N_UNDF*/ | 0x01 /*N_EXT*/,
        .n_sect = 0,
        .n_desc = 0,
        .n_value = 0,
    });
    for (auto pos : positions) {
      relocs.push_back({
          .r_address = static_cast<std::int32_t>(
              pos),  // after first byte address to someFuncExternal
          .r_symbolnum = sym_num,  // second symbol
          .r_pcrel = 1,            // relative call, PC counted
          .r_length = 2,           // 4 bytes
          .r_extern = 1,           // external
          .r_type = 2 /*GENERIC_RELOC_SECTDIFF*/,
      });
    }
    ++sym_num;
    sym_str_size += label.size() + 1;
  }

  std::uint32_t section_offset =
      sizeof(MachHeader64) + sizeof(SegmentCommand64) + sizeof(Section64) +
      sizeof(BuildVersionCommand) + sizeof(SymTabCommand) +
      sizeof(DySymTabCommand);
  std::uint32_t section_size = assembler.OutputBytesCount();

  std::uint32_t symtab_offset =
      section_offset + section_size + sizeof(RelocationInfo) * relocs.size();
  std::uint32_t symtab_size =
      sizeof(NList64) * (ext_def_syms.size() + undef_syms.size());

  std::uint32_t string_table_offset = symtab_offset + symtab_size;

  const DySymTabCommand dysym_tab{
      .ilocalsym = 0,
      .nlocalsym = 0,
      .iextdefsym = 0,
      .nextdefsym = static_cast<std::uint32_t>(ext_def_syms.size()),
      .iundefsym = static_cast<std::uint32_t>(ext_def_syms.size()),
      .nundefsym = static_cast<std::uint32_t>(undef_syms.size()),
      .tocoff = 0,
      .ntoc = 0,
      .modtaboff = 0,
      .nmodtab = 0,
      .extrefsymoff = 0,
      .nextrefsyms = 0,
      .indirectsymoff = 0,
      .nindirectsyms = 0,
      .extreloff = 0,
      .nextrel = 0,
      .locreloff = 0,
      .nlocrel = 0,
  };
  const SymTabCommand sym_tab{
      .symoff = symtab_offset,
      .nsyms =
          static_cast<std::uint32_t>(ext_def_syms.size() + undef_syms.size()),
      .stroff = string_table_offset,
      .strsize = sym_str_size,
  };
  const BuildVersionCommand build_version{
      .cmdsize = 24,
      .platform = 1,
      .minos = 0b00000000000011110000000000000000,
      .sdk = 0,
      .ntools = 0,
  };

  const Section64 section{
      .sectname = "__text",
      .segname = "__TEXT",
      .addr = 0,
      .size = section_size,
      .offset = section_offset,
      .align = 1 << 1,
      .reloff = section_offset + section_size,
      .nreloc = 2,
      .flags = kAttrPureInstructions | kAttrSomeInstructions,
  };
  const SegmentCommand64 segment{
      .cmdsize = sizeof(SegmentCommand64) + (sizeof(Section64) * 1),
      .segname = "",
      .vmaddr = 0,
      .vmsize = section_size,
      .fileoff = section_offset,
      .filesize = section_size,
      .maxprot = VmProt::ReadWriteExecute,
      .initprot = VmProt::ReadWriteExecute,
      .nsects = 1,
      .flags = 0,
  };
  const MachHeader64 header = {
      .cpu_type = CpuType::Arm64,
      .cpu_sub_type = CpuSubType::Arm64All,
      .file_type = FileType::Object,
      .ncmds = 3,
      .sizeofcmds = segment.cmdsize + build_version.cmdsize + sym_tab.cmdsize +
                    dysym_tab.cmdsize,
      .flags = 0,
  };
  WriteStruct(header, out);
  WriteStruct(segment, out);
  WriteStruct(section, out);
  WriteStruct(build_version, out);
  WriteStruct(sym_tab, out);
  WriteStruct(dysym_tab, out);
  assembler.WriteBytes(out);
  for (const auto& reloc : relocs) WriteStruct(reloc, out);
  for (const auto& sym : ext_def_syms) WriteStruct(sym, out);
  for (const auto& sym : undef_syms) WriteStruct(sym, out);

  out.put(0);
  for (const auto& [label, _] : assembler.GlobalLabels()) {
    out.write(label.data(), label.size());
    out.put(0);
  }
  for (const auto& [label, _] : assembler.ExternalLabels()) {
    out.write(label.data(), label.size());
    out.put(0);
  }
}

}  // namespace lucid
