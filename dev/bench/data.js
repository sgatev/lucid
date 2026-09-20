window.BENCHMARK_DATA = {
  "lastUpdate": 1789924661065,
  "repoUrl": "https://github.com/sgatev/lucid",
  "entries": {
    "Lucid benchmarks": [
      {
        "commit": {
          "author": {
            "email": "sg@sgatev.com",
            "name": "Stanislav Gatev",
            "username": "sgatev"
          },
          "committer": {
            "email": "sg@sgatev.com",
            "name": "Stanislav Gatev",
            "username": "sgatev"
          },
          "distinct": true,
          "id": "939034af3445fdd565c4d969cd8d5d0fb46be71a",
          "message": "Destroy the values a table is finished with",
          "timestamp": "2026-09-20T19:11:53+02:00",
          "tree_id": "16d0a0803f2d3f3835c0d09a88effc1034e277f5",
          "url": "https://github.com/sgatev/lucid/commit/939034af3445fdd565c4d969cd8d5d0fb46be71a"
        },
        "date": 1789924659549,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 54771.743,
            "unit": "ns/iter",
            "extra": "2382 iterations, 32.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 260912.892,
            "unit": "ns/iter",
            "extra": "530 iterations, 29.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 650507.005,
            "unit": "ns/iter",
            "extra": "220 iterations, 23.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 237187.566,
            "unit": "ns/iter",
            "extra": "636 iterations, 14.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 1817025.316,
            "unit": "ns/iter",
            "extra": "79 iterations, 4.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 6646692.476,
            "unit": "ns/iter",
            "extra": "21 iterations, 2.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 568072.115,
            "unit": "ns/iter",
            "extra": "234 iterations, 6.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 2086233.46,
            "unit": "ns/iter",
            "extra": "63 iterations, 3.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 2152459.615,
            "unit": "ns/iter",
            "extra": "65 iterations, 1.6MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 594.114,
            "unit": "ns/iter",
            "extra": "188018 iterations, 89.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 14423.17,
            "unit": "ns/iter",
            "extra": "9784 iterations, 529.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 27805.48,
            "unit": "ns/iter",
            "extra": "5122 iterations, 560.0MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 76765.21,
            "unit": "ns/iter",
            "extra": "1819 iterations, 70.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 148423.413,
            "unit": "ns/iter",
            "extra": "945 iterations, 72.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 9898.19,
            "unit": "ns/iter",
            "extra": "20000 iterations, 341.7MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 40.733,
            "unit": "ns/iter",
            "extra": "5682510 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 14.324,
            "unit": "ns/iter",
            "extra": "10301565 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 31.644,
            "unit": "ns/iter",
            "extra": "6344854 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 13.392,
            "unit": "ns/iter",
            "extra": "10140946 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 29.633,
            "unit": "ns/iter",
            "extra": "6852919 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 15.45,
            "unit": "ns/iter",
            "extra": "9738791 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 13.217,
            "unit": "ns/iter",
            "extra": "10057621 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 14.245,
            "unit": "ns/iter",
            "extra": "9946124 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 1.277,
            "unit": "ns/iter",
            "extra": "99570283 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 18.645,
            "unit": "ns/iter",
            "extra": "7534747 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 29.075,
            "unit": "ns/iter",
            "extra": "6547882 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 11,
            "unit": "ns/iter",
            "extra": "12111294 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 11.432,
            "unit": "ns/iter",
            "extra": "12950172 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 15.611,
            "unit": "ns/iter",
            "extra": "10613363 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 1.261,
            "unit": "ns/iter",
            "extra": "100000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 18.319,
            "unit": "ns/iter",
            "extra": "7870658 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 2.944,
            "unit": "ns/iter",
            "extra": "200000000 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.573,
            "unit": "ns/iter",
            "extra": "56342744 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 6.295,
            "unit": "ns/iter",
            "extra": "24285001 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 719713.96,
            "unit": "ns/iter",
            "extra": "200 iterations, 1456.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 735591.455,
            "unit": "ns/iter",
            "extra": "200 iterations, 1425.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 783647.705,
            "unit": "ns/iter",
            "extra": "200 iterations, 1337.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 716533.125,
            "unit": "ns/iter",
            "extra": "200 iterations, 1463.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 663064.967,
            "unit": "ns/iter",
            "extra": "211 iterations, 1581.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 646557.392,
            "unit": "ns/iter",
            "extra": "212 iterations, 1621.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 695784.074,
            "unit": "ns/iter",
            "extra": "203 iterations, 1506.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 1010137.562,
            "unit": "ns/iter",
            "extra": "146 iterations, 1032.0MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1996063.259,
            "unit": "ns/iter",
            "extra": "54 iterations, 525.3MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1838030.822,
            "unit": "ns/iter",
            "extra": "73 iterations, 570.5MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2874848.404,
            "unit": "ns/iter",
            "extra": "47 iterations, 364.7MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 4025201.389,
            "unit": "ns/iter",
            "extra": "36 iterations, 259.0MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 264152.519,
            "unit": "ns/iter",
            "extra": "642 iterations, 58.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 523295.717,
            "unit": "ns/iter",
            "extra": "247 iterations, 60.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 873370.835,
            "unit": "ns/iter",
            "extra": "200 iterations, 12.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 2088947.081,
            "unit": "ns/iter",
            "extra": "74 iterations, 10.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 1799700.586,
            "unit": "ns/iter",
            "extra": "70 iterations, 16.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 389566.827,
            "unit": "ns/iter",
            "extra": "727 iterations, 13.9MB/s"
          }
        ]
      }
    ]
  }
}