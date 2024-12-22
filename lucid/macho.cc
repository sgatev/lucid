#include "lucid/macho.h"

#include <cstdint>
#include <ostream>
#include <vector>

#include "lucid/arm64.h"

namespace lucid {
namespace {

using cpu_type_t = int;

constexpr static cpu_type_t kCpuArchAbi64 = 0x01000000;

constexpr static cpu_type_t kCpuTypeArm = 12;
constexpr static cpu_type_t kCpuTypeArm64 = kCpuTypeArm | kCpuArchAbi64;

using cpu_subtype_t = int;

constexpr static cpu_subtype_t kCpuSubtypeArm64All = 0;

using vm_prot_t = int;

// Read permission.
constexpr vm_prot_t kVmProtRead = 0x01;

// Write permission.
constexpr vm_prot_t kVmProtWrite = 0x02;

// Execute permission.
constexpr vm_prot_t kVmProtExecute = 0x04;

// Constant for the magic field of the mach_header_64 (64-bit architectures)
constexpr std::uint32_t kMhMagic64 = 0xfeedfacf;

// Constants for the filetype field of the mach_header_64.
constexpr std::uint32_t kMhObject = 0x1;

// Constants for the cmd field of new load commands, the type.
// 64-bit segment of this file to be mapped.
constexpr std::uint32_t kLcSymTab = 0x2;
constexpr std::uint32_t kLcSegment64 = 0x19;
constexpr std::uint32_t kLcBuildVersion = 0x32;

// Constants for the section attributes part of the flags field of a section
// structure.
// Section contains only true machine instructions.
constexpr std::uint32_t kAttrPureInstructions = 0x80000000;

// Section contains some machine instructions.
constexpr std::uint32_t kAttrSomeInstructions = 0x00000400;

struct mach_header_64 {
  std::uint32_t magic;
  cpu_type_t cputype;
  cpu_subtype_t cpusubtype;
  std::uint32_t filetype;
  std::uint32_t ncmds;
  std::uint32_t sizeofcmds;
  std::uint32_t flags;
  std::uint32_t reserved;
};

struct load_command {
  std::uint32_t cmd;
  std::uint32_t cmdsize;
};

struct segment_command_64 {
  std::uint32_t cmd;
  std::uint32_t cmdsize;
  char segname[16];
  std::uint64_t vmaddr;
  std::uint64_t vmsize;
  std::uint64_t fileoff;
  std::uint64_t filesize;
  vm_prot_t maxprot;
  vm_prot_t initprot;
  std::uint32_t nsects;
  std::uint32_t flags;
};

struct build_version_command {
  std::uint32_t cmd;
  std::uint32_t cmdsize;
  std::uint32_t platform;
  std::uint32_t minos;
  std::uint32_t sdk;
  std::uint32_t ntools;
};

struct symtab_command {
  std::uint32_t cmd;
  std::uint32_t cmdsize;
  std::uint32_t symoff;
  std::uint32_t nsyms;
  std::uint32_t stroff;
  std::uint32_t strsize;
};

struct section_64 {
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

struct nlist_64 {
  union {
    std::uint32_t n_strx; /* index into the string table */
  } n_un;
  std::uint8_t n_type;   /* type flag, see below */
  std::uint8_t n_sect;   /* section number or NO_SECT */
  std::uint16_t n_desc;  /* see <mach-o/stab.h> */
  std::uint64_t n_value; /* value of this symbol (or stab offset) */
};

}  // namespace

void AssembleMachObject(const arm64::Arm64& arm, std::ostream& out) {
  const std::vector<std::uint8_t> insts = arm.Encode();

  std::uint32_t section_offset =
      sizeof(mach_header_64) + sizeof(segment_command_64) + sizeof(section_64) +
      sizeof(build_version_command) + sizeof(symtab_command);
  std::uint32_t section_size = insts.size();

  std::uint32_t symtab_offset = section_offset + section_size;
  std::uint32_t symtab_size = sizeof(nlist_64);

  std::uint32_t string_table_offset = symtab_offset + symtab_size;

  const nlist_64 sym{
      .n_un{
          .n_strx = 1,
      },
      .n_type = 0xe /*N_SECt*/ | 0x01 /*N_EXT*/,
      .n_sect = 1,
      .n_desc = 0,
      .n_value = 0,
  };
  const symtab_command sym_tab{
      .cmd = kLcSymTab,
      .cmdsize = 24,
      .symoff = symtab_offset,
      .nsyms = 1,
      .stroff = string_table_offset,
      .strsize = 8,
  };
  const build_version_command build_version{
      .cmd = kLcBuildVersion,
      .cmdsize = 24,
      .platform = 1,
      .minos = 0b00000000000011110000000000000000,
      .sdk = 0,
      .ntools = 0,
  };
  const section_64 section{
      .sectname = "__text",
      .segname = "__TEXT",
      .addr = 0,
      .size = section_size,
      .offset = section_offset,
      .align = 1 << 1,
      .reloff = 0,
      .nreloc = 0,
      .flags = kAttrPureInstructions | kAttrSomeInstructions,
  };
  const segment_command_64 segment{
      .cmd = kLcSegment64,
      .cmdsize = sizeof(segment_command_64) + (sizeof(section_64) * 1),
      .segname = "",
      .vmaddr = 0,
      .vmsize = section_size,
      .fileoff = section_offset,
      .filesize = section_size,
      .maxprot = kVmProtRead | kVmProtWrite | kVmProtExecute,
      .initprot = kVmProtRead | kVmProtWrite | kVmProtExecute,
      .nsects = 1,
      .flags = 0,
  };
  const mach_header_64 header = {
      .magic = kMhMagic64,
      .cputype = kCpuTypeArm64,
      .cpusubtype = kCpuSubtypeArm64All,
      .filetype = kMhObject,
      .ncmds = 3,
      .sizeofcmds = segment.cmdsize + build_version.cmdsize + sym_tab.cmdsize,
      .flags = 0,
  };
  out.write(reinterpret_cast<const char*>(&header), sizeof(header));
  out.write(reinterpret_cast<const char*>(&segment), sizeof(segment));
  out.write(reinterpret_cast<const char*>(&section), sizeof(section));
  out.write(reinterpret_cast<const char*>(&build_version),
            sizeof(build_version));
  out.write(reinterpret_cast<const char*>(&sym_tab), sizeof(sym_tab));
  for (auto inst : insts) {
    out.write(reinterpret_cast<const char*>(&inst), 1);
  }
  out.write(reinterpret_cast<const char*>(&sym), sizeof(sym));

  out.write("\0", 1);
  out.write("_start", 6);
  out.write("\0", 1);
}

}  // namespace lucid
