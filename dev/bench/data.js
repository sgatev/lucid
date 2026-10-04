window.BENCHMARK_DATA = {
  "lastUpdate": 1791127017341,
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
      },
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
          "id": "ff045e80af2b0fcd809663ccbcbbe9de85a36f0d",
          "message": "Read the benchmark results on a page with a menu",
          "timestamp": "2026-09-20T19:31:43+02:00",
          "tree_id": "1a7efc277f3da3c254dd6195a1ee3e5b869abcb8",
          "url": "https://github.com/sgatev/lucid/commit/ff045e80af2b0fcd809663ccbcbbe9de85a36f0d"
        },
        "date": 1789926113260,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 57676.702,
            "unit": "ns/iter",
            "extra": "2458 iterations, 31.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 263109.924,
            "unit": "ns/iter",
            "extra": "503 iterations, 29.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 579995.013,
            "unit": "ns/iter",
            "extra": "234 iterations, 26.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 221791.402,
            "unit": "ns/iter",
            "extra": "627 iterations, 15.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 1677972.561,
            "unit": "ns/iter",
            "extra": "82 iterations, 4.6MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 6179232.955,
            "unit": "ns/iter",
            "extra": "22 iterations, 2.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 524663.232,
            "unit": "ns/iter",
            "extra": "267 iterations, 6.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 2008713.94,
            "unit": "ns/iter",
            "extra": "67 iterations, 3.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 2083191.288,
            "unit": "ns/iter",
            "extra": "66 iterations, 1.6MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 558.467,
            "unit": "ns/iter",
            "extra": "193606 iterations, 94.9MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 14743.611,
            "unit": "ns/iter",
            "extra": "9398 iterations, 517.9MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 28402.971,
            "unit": "ns/iter",
            "extra": "4536 iterations, 548.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 73621.561,
            "unit": "ns/iter",
            "extra": "1890 iterations, 73.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 148452.043,
            "unit": "ns/iter",
            "extra": "934 iterations, 72.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 10085.467,
            "unit": "ns/iter",
            "extra": "10000 iterations, 335.3MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 35.601,
            "unit": "ns/iter",
            "extra": "5560741 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 12.94,
            "unit": "ns/iter",
            "extra": "9872654 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 31.383,
            "unit": "ns/iter",
            "extra": "6265862 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 13.6,
            "unit": "ns/iter",
            "extra": "10717874 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 31.678,
            "unit": "ns/iter",
            "extra": "3362515 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 13.33,
            "unit": "ns/iter",
            "extra": "10447534 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 14.861,
            "unit": "ns/iter",
            "extra": "9632999 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 15.756,
            "unit": "ns/iter",
            "extra": "8684661 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 1.331,
            "unit": "ns/iter",
            "extra": "100000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 19.561,
            "unit": "ns/iter",
            "extra": "7106328 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 31.142,
            "unit": "ns/iter",
            "extra": "5954100 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 10.853,
            "unit": "ns/iter",
            "extra": "14708714 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 9.983,
            "unit": "ns/iter",
            "extra": "12710853 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 15.16,
            "unit": "ns/iter",
            "extra": "9765710 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 1.264,
            "unit": "ns/iter",
            "extra": "100000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 21.233,
            "unit": "ns/iter",
            "extra": "8384384 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 2.467,
            "unit": "ns/iter",
            "extra": "200000000 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.495,
            "unit": "ns/iter",
            "extra": "57650724 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 6.02,
            "unit": "ns/iter",
            "extra": "22450288 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 679486.779,
            "unit": "ns/iter",
            "extra": "208 iterations, 1543.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 681581.264,
            "unit": "ns/iter",
            "extra": "201 iterations, 1538.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 754843.75,
            "unit": "ns/iter",
            "extra": "200 iterations, 1389.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 711943.545,
            "unit": "ns/iter",
            "extra": "200 iterations, 1472.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 656919.991,
            "unit": "ns/iter",
            "extra": "213 iterations, 1596.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 641544.548,
            "unit": "ns/iter",
            "extra": "217 iterations, 1634.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 683675.563,
            "unit": "ns/iter",
            "extra": "206 iterations, 1533.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 754198.123,
            "unit": "ns/iter",
            "extra": "204 iterations, 1382.2MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1767924.354,
            "unit": "ns/iter",
            "extra": "65 iterations, 593.1MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1878590.178,
            "unit": "ns/iter",
            "extra": "73 iterations, 558.2MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2933336.812,
            "unit": "ns/iter",
            "extra": "48 iterations, 357.4MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 3900032.417,
            "unit": "ns/iter",
            "extra": "36 iterations, 267.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 265991.207,
            "unit": "ns/iter",
            "extra": "564 iterations, 58.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 500471.296,
            "unit": "ns/iter",
            "extra": "270 iterations, 63.0MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 836208.545,
            "unit": "ns/iter",
            "extra": "200 iterations, 12.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 1970008.681,
            "unit": "ns/iter",
            "extra": "72 iterations, 10.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 1793837.349,
            "unit": "ns/iter",
            "extra": "83 iterations, 16.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 282276.273,
            "unit": "ns/iter",
            "extra": "517 iterations, 19.1MB/s"
          }
        ]
      },
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
          "id": "a2e502b6d2d58e9b2703f4a096933d25f7997297",
          "message": "Forget the emptied slots a table leaves behind",
          "timestamp": "2026-09-20T19:55:33+02:00",
          "tree_id": "42008288edca782669815676242f594da8088cea",
          "url": "https://github.com/sgatev/lucid/commit/a2e502b6d2d58e9b2703f4a096933d25f7997297"
        },
        "date": 1789927264848,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 63111.364,
            "unit": "ns/iter",
            "extra": "2139 iterations, 28.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 304557.324,
            "unit": "ns/iter",
            "extra": "479 iterations, 25.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 656718.96,
            "unit": "ns/iter",
            "extra": "200 iterations, 23.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 310852.36,
            "unit": "ns/iter",
            "extra": "392 iterations, 10.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 1195993.613,
            "unit": "ns/iter",
            "extra": "150 iterations, 6.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 3822272.727,
            "unit": "ns/iter",
            "extra": "33 iterations, 4.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 433595.954,
            "unit": "ns/iter",
            "extra": "373 iterations, 7.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 1861316.576,
            "unit": "ns/iter",
            "extra": "92 iterations, 4.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 2665178.191,
            "unit": "ns/iter",
            "extra": "47 iterations, 1.3MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 784.017,
            "unit": "ns/iter",
            "extra": "133549 iterations, 67.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 17191.552,
            "unit": "ns/iter",
            "extra": "8827 iterations, 444.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 29532.314,
            "unit": "ns/iter",
            "extra": "4121 iterations, 527.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 76654.461,
            "unit": "ns/iter",
            "extra": "1857 iterations, 70.7MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 155984.679,
            "unit": "ns/iter",
            "extra": "892 iterations, 68.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 10021.013,
            "unit": "ns/iter",
            "extra": "10000 iterations, 337.5MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 39.589,
            "unit": "ns/iter",
            "extra": "5876447 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 14.659,
            "unit": "ns/iter",
            "extra": "7952568 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 34.932,
            "unit": "ns/iter",
            "extra": "5686241 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 17.126,
            "unit": "ns/iter",
            "extra": "8273722 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 42.419,
            "unit": "ns/iter",
            "extra": "5450466 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 13.835,
            "unit": "ns/iter",
            "extra": "9426789 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 14.55,
            "unit": "ns/iter",
            "extra": "12146846 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 14.704,
            "unit": "ns/iter",
            "extra": "9730527 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 1.258,
            "unit": "ns/iter",
            "extra": "100000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 18.401,
            "unit": "ns/iter",
            "extra": "8648715 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 31.409,
            "unit": "ns/iter",
            "extra": "6154793 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 10.801,
            "unit": "ns/iter",
            "extra": "12485368 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 11.809,
            "unit": "ns/iter",
            "extra": "12534226 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 15.293,
            "unit": "ns/iter",
            "extra": "9588740 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 1.279,
            "unit": "ns/iter",
            "extra": "100000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 18.943,
            "unit": "ns/iter",
            "extra": "7291809 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 1.062,
            "unit": "ns/iter",
            "extra": "100000000 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.658,
            "unit": "ns/iter",
            "extra": "58572308 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 6.362,
            "unit": "ns/iter",
            "extra": "22574422 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 739496.809,
            "unit": "ns/iter",
            "extra": "209 iterations, 1417.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 726786.585,
            "unit": "ns/iter",
            "extra": "205 iterations, 1442.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 745839.165,
            "unit": "ns/iter",
            "extra": "200 iterations, 1405.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 775151.67,
            "unit": "ns/iter",
            "extra": "200 iterations, 1352.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 665911.679,
            "unit": "ns/iter",
            "extra": "209 iterations, 1574.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 642852.951,
            "unit": "ns/iter",
            "extra": "223 iterations, 1631.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 675100.81,
            "unit": "ns/iter",
            "extra": "205 iterations, 1553.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 753997.5,
            "unit": "ns/iter",
            "extra": "200 iterations, 1382.5MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1847868.75,
            "unit": "ns/iter",
            "extra": "60 iterations, 567.4MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1803400.641,
            "unit": "ns/iter",
            "extra": "78 iterations, 581.4MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2834328.125,
            "unit": "ns/iter",
            "extra": "48 iterations, 369.9MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 3762215.081,
            "unit": "ns/iter",
            "extra": "37 iterations, 277.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 290239.693,
            "unit": "ns/iter",
            "extra": "566 iterations, 53.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 676252.836,
            "unit": "ns/iter",
            "extra": "250 iterations, 46.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 1071302.92,
            "unit": "ns/iter",
            "extra": "100 iterations, 10.0MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 2338943.966,
            "unit": "ns/iter",
            "extra": "58 iterations, 9.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 2300199.87,
            "unit": "ns/iter",
            "extra": "69 iterations, 12.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 303521.435,
            "unit": "ns/iter",
            "extra": "519 iterations, 17.8MB/s"
          }
        ]
      },
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
          "id": "4114d97d0bd6df1129440b4c8e4b268ccecce44f",
          "message": "Read a block's live-out off the blocks after it",
          "timestamp": "2026-09-20T21:17:14+02:00",
          "tree_id": "4e384f3b3bd473694d6211ca70489e1cbd896ca3",
          "url": "https://github.com/sgatev/lucid/commit/4114d97d0bd6df1129440b4c8e4b268ccecce44f"
        },
        "date": 1789932172369,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 123133.502,
            "unit": "ns/iter",
            "extra": "1539 iterations, 14.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 443772.166,
            "unit": "ns/iter",
            "extra": "391 iterations, 17.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 951690,
            "unit": "ns/iter",
            "extra": "200 iterations, 16.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 298990.694,
            "unit": "ns/iter",
            "extra": "385 iterations, 11.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 1400274.16,
            "unit": "ns/iter",
            "extra": "100 iterations, 5.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 5407983.576,
            "unit": "ns/iter",
            "extra": "33 iterations, 2.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 528328.323,
            "unit": "ns/iter",
            "extra": "316 iterations, 6.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 1947096.2,
            "unit": "ns/iter",
            "extra": "55 iterations, 3.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 3165404.16,
            "unit": "ns/iter",
            "extra": "50 iterations, 1.1MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 874.441,
            "unit": "ns/iter",
            "extra": "193732 iterations, 60.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 20032.275,
            "unit": "ns/iter",
            "extra": "5927 iterations, 381.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 34034.503,
            "unit": "ns/iter",
            "extra": "4380 iterations, 457.5MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 95720.185,
            "unit": "ns/iter",
            "extra": "1663 iterations, 56.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 201674.782,
            "unit": "ns/iter",
            "extra": "724 iterations, 53.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 11413.246,
            "unit": "ns/iter",
            "extra": "10000 iterations, 296.3MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 51.295,
            "unit": "ns/iter",
            "extra": "5237413 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 21.561,
            "unit": "ns/iter",
            "extra": "5271034 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 36.393,
            "unit": "ns/iter",
            "extra": "3502320 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 21.229,
            "unit": "ns/iter",
            "extra": "8979395 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 46.391,
            "unit": "ns/iter",
            "extra": "5182346 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 33.311,
            "unit": "ns/iter",
            "extra": "8059216 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 22.771,
            "unit": "ns/iter",
            "extra": "6095072 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 21.321,
            "unit": "ns/iter",
            "extra": "6189395 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 1.913,
            "unit": "ns/iter",
            "extra": "89238306 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 29.431,
            "unit": "ns/iter",
            "extra": "7444258 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 51.952,
            "unit": "ns/iter",
            "extra": "4345835 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 18.542,
            "unit": "ns/iter",
            "extra": "6567823 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 25.799,
            "unit": "ns/iter",
            "extra": "7543602 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 19.127,
            "unit": "ns/iter",
            "extra": "5738381 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 1.871,
            "unit": "ns/iter",
            "extra": "100000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 27.219,
            "unit": "ns/iter",
            "extra": "5150672 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 1.362,
            "unit": "ns/iter",
            "extra": "100000000 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 3.724,
            "unit": "ns/iter",
            "extra": "47142682 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 7.654,
            "unit": "ns/iter",
            "extra": "21436637 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 906855.835,
            "unit": "ns/iter",
            "extra": "200 iterations, 1156.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 941252.5,
            "unit": "ns/iter",
            "extra": "200 iterations, 1114.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 1131637.5,
            "unit": "ns/iter",
            "extra": "100 iterations, 926.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 1038685.83,
            "unit": "ns/iter",
            "extra": "100 iterations, 1009.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 874681.665,
            "unit": "ns/iter",
            "extra": "200 iterations, 1198.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 898135.835,
            "unit": "ns/iter",
            "extra": "200 iterations, 1167.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 849663.081,
            "unit": "ns/iter",
            "extra": "186 iterations, 1233.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 1074793.396,
            "unit": "ns/iter",
            "extra": "96 iterations, 969.9MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 2544672.551,
            "unit": "ns/iter",
            "extra": "78 iterations, 412.1MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 2596967.593,
            "unit": "ns/iter",
            "extra": "54 iterations, 403.8MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 3939792.882,
            "unit": "ns/iter",
            "extra": "34 iterations, 266.1MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 6185419.517,
            "unit": "ns/iter",
            "extra": "29 iterations, 168.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 424439.086,
            "unit": "ns/iter",
            "extra": "394 iterations, 36.7MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 828125.625,
            "unit": "ns/iter",
            "extra": "200 iterations, 38.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 1040791.66,
            "unit": "ns/iter",
            "extra": "100 iterations, 10.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 3016708.333,
            "unit": "ns/iter",
            "extra": "66 iterations, 7.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 2511488.183,
            "unit": "ns/iter",
            "extra": "60 iterations, 11.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 330557.121,
            "unit": "ns/iter",
            "extra": "612 iterations, 16.4MB/s"
          }
        ]
      },
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
          "id": "e33244732e3704972e57da6f4da4709f19bfeba2",
          "message": "Mix hashes together rather than fold them with xor",
          "timestamp": "2026-09-20T22:04:22+02:00",
          "tree_id": "e45077ba037636df10935f4ba637c7051cc13bda",
          "url": "https://github.com/sgatev/lucid/commit/e33244732e3704972e57da6f4da4709f19bfeba2"
        },
        "date": 1789935040701,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 90490.651,
            "unit": "ns/iter",
            "extra": "1707 iterations, 19.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 523728.768,
            "unit": "ns/iter",
            "extra": "418 iterations, 14.6MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 1540741.015,
            "unit": "ns/iter",
            "extra": "65 iterations, 10.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 388838.738,
            "unit": "ns/iter",
            "extra": "370 iterations, 8.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 349737.005,
            "unit": "ns/iter",
            "extra": "404 iterations, 21.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 769438.125,
            "unit": "ns/iter",
            "extra": "200 iterations, 20.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 323132.367,
            "unit": "ns/iter",
            "extra": "526 iterations, 10.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 807243.545,
            "unit": "ns/iter",
            "extra": "200 iterations, 9.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 3343613.9,
            "unit": "ns/iter",
            "extra": "60 iterations, 1.0MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 868.842,
            "unit": "ns/iter",
            "extra": "171101 iterations, 61.0MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 19598.98,
            "unit": "ns/iter",
            "extra": "7805 iterations, 389.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 39040.075,
            "unit": "ns/iter",
            "extra": "4948 iterations, 398.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 166265.078,
            "unit": "ns/iter",
            "extra": "1296 iterations, 32.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 199365.82,
            "unit": "ns/iter",
            "extra": "640 iterations, 53.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 12141.417,
            "unit": "ns/iter",
            "extra": "10000 iterations, 278.6MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 44.711,
            "unit": "ns/iter",
            "extra": "3853462 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 20.247,
            "unit": "ns/iter",
            "extra": "11427715 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 50.288,
            "unit": "ns/iter",
            "extra": "5989966 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 21.098,
            "unit": "ns/iter",
            "extra": "8251959 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 44.62,
            "unit": "ns/iter",
            "extra": "5771847 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 20.71,
            "unit": "ns/iter",
            "extra": "8377256 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 17.396,
            "unit": "ns/iter",
            "extra": "9716008 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 17.409,
            "unit": "ns/iter",
            "extra": "7486347 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 1.677,
            "unit": "ns/iter",
            "extra": "100000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 24.608,
            "unit": "ns/iter",
            "extra": "5437905 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 48.274,
            "unit": "ns/iter",
            "extra": "4604442 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 19.205,
            "unit": "ns/iter",
            "extra": "6327933 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 15.257,
            "unit": "ns/iter",
            "extra": "11681308 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 19.229,
            "unit": "ns/iter",
            "extra": "8237030 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 2.259,
            "unit": "ns/iter",
            "extra": "100000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 31.47,
            "unit": "ns/iter",
            "extra": "7086036 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 1.349,
            "unit": "ns/iter",
            "extra": "100000000 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 5.32,
            "unit": "ns/iter",
            "extra": "47185709 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 6.925,
            "unit": "ns/iter",
            "extra": "18961672 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 917307.5,
            "unit": "ns/iter",
            "extra": "200 iterations, 1143.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 951526.04,
            "unit": "ns/iter",
            "extra": "200 iterations, 1101.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 959851.46,
            "unit": "ns/iter",
            "extra": "200 iterations, 1092.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 1188213.75,
            "unit": "ns/iter",
            "extra": "200 iterations, 882.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 952028.75,
            "unit": "ns/iter",
            "extra": "200 iterations, 1101.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 831806.46,
            "unit": "ns/iter",
            "extra": "200 iterations, 1260.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 1597298.818,
            "unit": "ns/iter",
            "extra": "99 iterations, 656.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 1092383.335,
            "unit": "ns/iter",
            "extra": "200 iterations, 954.3MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 3036143.263,
            "unit": "ns/iter",
            "extra": "57 iterations, 345.4MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 2720341.51,
            "unit": "ns/iter",
            "extra": "51 iterations, 385.5MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 3843478.125,
            "unit": "ns/iter",
            "extra": "40 iterations, 272.8MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 5032896.593,
            "unit": "ns/iter",
            "extra": "27 iterations, 207.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 244426.355,
            "unit": "ns/iter",
            "extra": "830 iterations, 63.7MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 489451.35,
            "unit": "ns/iter",
            "extra": "340 iterations, 64.4MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 1025649.16,
            "unit": "ns/iter",
            "extra": "100 iterations, 10.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 2403777.062,
            "unit": "ns/iter",
            "extra": "97 iterations, 8.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 1585128.75,
            "unit": "ns/iter",
            "extra": "100 iterations, 18.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 192889.886,
            "unit": "ns/iter",
            "extra": "753 iterations, 28.0MB/s"
          }
        ]
      },
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
          "id": "6b4a1d91b1809b9e1520dd4e7f622727db2ab11d",
          "message": "Search past an emptied slot before taking it",
          "timestamp": "2026-09-21T21:29:12+02:00",
          "tree_id": "0e6382033842c4862a6f349bb359d36d49d5cd32",
          "url": "https://github.com/sgatev/lucid/commit/6b4a1d91b1809b9e1520dd4e7f622727db2ab11d"
        },
        "date": 1790019305825,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 63974.264,
            "unit": "ns/iter",
            "extra": "1980 iterations, 28.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 296678.986,
            "unit": "ns/iter",
            "extra": "416 iterations, 25.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 660193.821,
            "unit": "ns/iter",
            "extra": "224 iterations, 23.6MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 355043.456,
            "unit": "ns/iter",
            "extra": "792 iterations, 9.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 251323.504,
            "unit": "ns/iter",
            "extra": "568 iterations, 30.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 597335.211,
            "unit": "ns/iter",
            "extra": "266 iterations, 26.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 222689.858,
            "unit": "ns/iter",
            "extra": "548 iterations, 15.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 636322.338,
            "unit": "ns/iter",
            "extra": "216 iterations, 12.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 1928879.897,
            "unit": "ns/iter",
            "extra": "68 iterations, 1.8MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 594.28,
            "unit": "ns/iter",
            "extra": "171925 iterations, 89.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 14957.868,
            "unit": "ns/iter",
            "extra": "9320 iterations, 510.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 28760.799,
            "unit": "ns/iter",
            "extra": "4653 iterations, 541.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 77123.758,
            "unit": "ns/iter",
            "extra": "1845 iterations, 70.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 153638.158,
            "unit": "ns/iter",
            "extra": "931 iterations, 69.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 10347.523,
            "unit": "ns/iter",
            "extra": "20000 iterations, 326.8MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 39.332,
            "unit": "ns/iter",
            "extra": "4914479 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 13.206,
            "unit": "ns/iter",
            "extra": "9604170 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 32.918,
            "unit": "ns/iter",
            "extra": "6331916 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 13.503,
            "unit": "ns/iter",
            "extra": "10745020 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 33.863,
            "unit": "ns/iter",
            "extra": "5833829 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 14.968,
            "unit": "ns/iter",
            "extra": "9831604 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 12.089,
            "unit": "ns/iter",
            "extra": "9989267 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 13.578,
            "unit": "ns/iter",
            "extra": "10496489 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 1.336,
            "unit": "ns/iter",
            "extra": "100000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 18.14,
            "unit": "ns/iter",
            "extra": "8385472 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 46.63,
            "unit": "ns/iter",
            "extra": "5851986 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 13.602,
            "unit": "ns/iter",
            "extra": "8816398 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 14.88,
            "unit": "ns/iter",
            "extra": "10522755 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 15.673,
            "unit": "ns/iter",
            "extra": "7608196 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 1.372,
            "unit": "ns/iter",
            "extra": "100000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 17.804,
            "unit": "ns/iter",
            "extra": "7742095 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 4.29,
            "unit": "ns/iter",
            "extra": "200000000 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.569,
            "unit": "ns/iter",
            "extra": "56587559 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 6.25,
            "unit": "ns/iter",
            "extra": "25340513 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 688666.279,
            "unit": "ns/iter",
            "extra": "215 iterations, 1522.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 954827.801,
            "unit": "ns/iter",
            "extra": "211 iterations, 1098.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 859503.75,
            "unit": "ns/iter",
            "extra": "200 iterations, 1219.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 716379.165,
            "unit": "ns/iter",
            "extra": "200 iterations, 1463.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 661164.876,
            "unit": "ns/iter",
            "extra": "209 iterations, 1585.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 964644.186,
            "unit": "ns/iter",
            "extra": "215 iterations, 1086.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 1005662.5,
            "unit": "ns/iter",
            "extra": "100 iterations, 1042.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 774255.835,
            "unit": "ns/iter",
            "extra": "200 iterations, 1346.3MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1931175,
            "unit": "ns/iter",
            "extra": "70 iterations, 543.0MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 2221818.945,
            "unit": "ns/iter",
            "extra": "55 iterations, 471.9MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2883067.837,
            "unit": "ns/iter",
            "extra": "43 iterations, 363.7MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 3807337.972,
            "unit": "ns/iter",
            "extra": "36 iterations, 273.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 136396.568,
            "unit": "ns/iter",
            "extra": "1304 iterations, 114.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 283157.105,
            "unit": "ns/iter",
            "extra": "610 iterations, 111.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 374317.42,
            "unit": "ns/iter",
            "extra": "398 iterations, 28.7MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 952048.125,
            "unit": "ns/iter",
            "extra": "200 iterations, 22.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 901361.04,
            "unit": "ns/iter",
            "extra": "200 iterations, 32.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 118924.017,
            "unit": "ns/iter",
            "extra": "1213 iterations, 45.4MB/s"
          }
        ]
      },
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
          "id": "bcab44723d35736eba9b5a676d1e2c0d6c83baad",
          "message": "Colour the registers a block's phi functions name",
          "timestamp": "2026-09-21T22:24:57+02:00",
          "tree_id": "a2f8ce5822ce29273794fe4aad796a6717c80dd7",
          "url": "https://github.com/sgatev/lucid/commit/bcab44723d35736eba9b5a676d1e2c0d6c83baad"
        },
        "date": 1790022717182,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 65608.198,
            "unit": "ns/iter",
            "extra": "2470 iterations, 27.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 260611.062,
            "unit": "ns/iter",
            "extra": "568 iterations, 29.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 593745.029,
            "unit": "ns/iter",
            "extra": "243 iterations, 26.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 232734.365,
            "unit": "ns/iter",
            "extra": "581 iterations, 14.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 240313.196,
            "unit": "ns/iter",
            "extra": "598 iterations, 31.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 513780.168,
            "unit": "ns/iter",
            "extra": "250 iterations, 30.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 195459.304,
            "unit": "ns/iter",
            "extra": "644 iterations, 17.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 611331.146,
            "unit": "ns/iter",
            "extra": "267 iterations, 12.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 1927100.352,
            "unit": "ns/iter",
            "extra": "71 iterations, 1.8MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 523.751,
            "unit": "ns/iter",
            "extra": "201829 iterations, 101.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 13743.408,
            "unit": "ns/iter",
            "extra": "10000 iterations, 555.5MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 43214.197,
            "unit": "ns/iter",
            "extra": "5244 iterations, 360.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 77257.456,
            "unit": "ns/iter",
            "extra": "1861 iterations, 70.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 155046.483,
            "unit": "ns/iter",
            "extra": "917 iterations, 69.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 9792.946,
            "unit": "ns/iter",
            "extra": "20000 iterations, 345.4MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 38.047,
            "unit": "ns/iter",
            "extra": "4856014 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 13.287,
            "unit": "ns/iter",
            "extra": "10782743 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 34.207,
            "unit": "ns/iter",
            "extra": "5907744 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 13.758,
            "unit": "ns/iter",
            "extra": "9414032 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 32.479,
            "unit": "ns/iter",
            "extra": "6163000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 14.897,
            "unit": "ns/iter",
            "extra": "9954080 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 12.924,
            "unit": "ns/iter",
            "extra": "10359466 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 13.142,
            "unit": "ns/iter",
            "extra": "10377577 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 1.256,
            "unit": "ns/iter",
            "extra": "100000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 17.281,
            "unit": "ns/iter",
            "extra": "8090809 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 32.475,
            "unit": "ns/iter",
            "extra": "6063887 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 9.764,
            "unit": "ns/iter",
            "extra": "12202119 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 9.023,
            "unit": "ns/iter",
            "extra": "19147807 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 13.689,
            "unit": "ns/iter",
            "extra": "10066329 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 1.596,
            "unit": "ns/iter",
            "extra": "100000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 17.502,
            "unit": "ns/iter",
            "extra": "7793653 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 2.957,
            "unit": "ns/iter",
            "extra": "200000000 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.467,
            "unit": "ns/iter",
            "extra": "54303914 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 6.003,
            "unit": "ns/iter",
            "extra": "23008813 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 669244.355,
            "unit": "ns/iter",
            "extra": "214 iterations, 1566.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 681584.756,
            "unit": "ns/iter",
            "extra": "205 iterations, 1538.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 746148.75,
            "unit": "ns/iter",
            "extra": "200 iterations, 1405.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 708318.33,
            "unit": "ns/iter",
            "extra": "200 iterations, 1480.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 661240.413,
            "unit": "ns/iter",
            "extra": "213 iterations, 1585.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 658756.441,
            "unit": "ns/iter",
            "extra": "220 iterations, 1591.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 670948.626,
            "unit": "ns/iter",
            "extra": "206 iterations, 1562.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 733175.835,
            "unit": "ns/iter",
            "extra": "200 iterations, 1421.8MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1894625.677,
            "unit": "ns/iter",
            "extra": "62 iterations, 553.4MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 2753280.667,
            "unit": "ns/iter",
            "extra": "72 iterations, 380.8MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2857427.778,
            "unit": "ns/iter",
            "extra": "45 iterations, 366.9MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 3579052.083,
            "unit": "ns/iter",
            "extra": "36 iterations, 291.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 134691.754,
            "unit": "ns/iter",
            "extra": "1425 iterations, 115.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 255778.975,
            "unit": "ns/iter",
            "extra": "673 iterations, 123.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 251729.984,
            "unit": "ns/iter",
            "extra": "612 iterations, 42.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 523660.651,
            "unit": "ns/iter",
            "extra": "284 iterations, 40.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 635081.986,
            "unit": "ns/iter",
            "extra": "278 iterations, 45.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 103608.979,
            "unit": "ns/iter",
            "extra": "1550 iterations, 52.2MB/s"
          }
        ]
      },
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
          "id": "4142ec52b8ac9dfa2d298bd187e3152a06f730eb",
          "message": "Keep a hash table half empty",
          "timestamp": "2026-09-23T06:54:16+02:00",
          "tree_id": "ddb0948c092371984f16342d559e4c82fb0727da",
          "url": "https://github.com/sgatev/lucid/commit/4142ec52b8ac9dfa2d298bd187e3152a06f730eb"
        },
        "date": 1790139593895,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 63615.832,
            "unit": "ns/iter",
            "extra": "2136 iterations, 28.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 285192.602,
            "unit": "ns/iter",
            "extra": "490 iterations, 26.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 574645.576,
            "unit": "ns/iter",
            "extra": "243 iterations, 27.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 260435.565,
            "unit": "ns/iter",
            "extra": "538 iterations, 13.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 151297.657,
            "unit": "ns/iter",
            "extra": "932 iterations, 50.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 309059.652,
            "unit": "ns/iter",
            "extra": "468 iterations, 50.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 147912.671,
            "unit": "ns/iter",
            "extra": "949 iterations, 22.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 497340.869,
            "unit": "ns/iter",
            "extra": "282 iterations, 15.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 1611455.28,
            "unit": "ns/iter",
            "extra": "82 iterations, 2.1MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 601.501,
            "unit": "ns/iter",
            "extra": "342562 iterations, 88.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 14329.802,
            "unit": "ns/iter",
            "extra": "9262 iterations, 532.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 27998.371,
            "unit": "ns/iter",
            "extra": "5040 iterations, 556.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 73557.32,
            "unit": "ns/iter",
            "extra": "1645 iterations, 73.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 189723.544,
            "unit": "ns/iter",
            "extra": "967 iterations, 56.5MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 9418.235,
            "unit": "ns/iter",
            "extra": "20000 iterations, 359.1MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 57.37,
            "unit": "ns/iter",
            "extra": "8452043 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 19.931,
            "unit": "ns/iter",
            "extra": "7424462 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 36.691,
            "unit": "ns/iter",
            "extra": "10763262 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 18.405,
            "unit": "ns/iter",
            "extra": "7334434 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 39.904,
            "unit": "ns/iter",
            "extra": "12038609 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 24.8,
            "unit": "ns/iter",
            "extra": "7244345 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 14.829,
            "unit": "ns/iter",
            "extra": "9546946 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 5.391,
            "unit": "ns/iter",
            "extra": "28820668 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 1.77,
            "unit": "ns/iter",
            "extra": "79193000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 6.782,
            "unit": "ns/iter",
            "extra": "23407128 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 37.572,
            "unit": "ns/iter",
            "extra": "9018124 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 16.231,
            "unit": "ns/iter",
            "extra": "9395421 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 11.113,
            "unit": "ns/iter",
            "extra": "14753277 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 5.49,
            "unit": "ns/iter",
            "extra": "29213834 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 1.768,
            "unit": "ns/iter",
            "extra": "70539343 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 6.706,
            "unit": "ns/iter",
            "extra": "19188486 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 3.246,
            "unit": "ns/iter",
            "extra": "200000000 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.333,
            "unit": "ns/iter",
            "extra": "57982034 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 5.814,
            "unit": "ns/iter",
            "extra": "24904938 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 650252.222,
            "unit": "ns/iter",
            "extra": "225 iterations, 1612.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 681694.379,
            "unit": "ns/iter",
            "extra": "206 iterations, 1538.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 749550.83,
            "unit": "ns/iter",
            "extra": "200 iterations, 1398.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 694020.415,
            "unit": "ns/iter",
            "extra": "200 iterations, 1510.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 654082.167,
            "unit": "ns/iter",
            "extra": "215 iterations, 1603.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 638884.555,
            "unit": "ns/iter",
            "extra": "218 iterations, 1641.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 666141.786,
            "unit": "ns/iter",
            "extra": "206 iterations, 1573.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 737082.927,
            "unit": "ns/iter",
            "extra": "206 iterations, 1414.2MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1918143.05,
            "unit": "ns/iter",
            "extra": "60 iterations, 546.7MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 2265588.115,
            "unit": "ns/iter",
            "extra": "61 iterations, 462.8MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 3292708.333,
            "unit": "ns/iter",
            "extra": "42 iterations, 318.4MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 4099412.879,
            "unit": "ns/iter",
            "extra": "33 iterations, 254.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 114386.937,
            "unit": "ns/iter",
            "extra": "1686 iterations, 136.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 280111.989,
            "unit": "ns/iter",
            "extra": "759 iterations, 112.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 248401.927,
            "unit": "ns/iter",
            "extra": "588 iterations, 43.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 598966.86,
            "unit": "ns/iter",
            "extra": "215 iterations, 35.7MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 855296.655,
            "unit": "ns/iter",
            "extra": "284 iterations, 34.0MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 112273.526,
            "unit": "ns/iter",
            "extra": "1509 iterations, 48.1MB/s"
          }
        ]
      },
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
          "id": "4a16f2a89e8641b510f7548462dfbffcb5ba048b",
          "message": "Spill the register read furthest ahead",
          "timestamp": "2026-09-24T06:37:15+02:00",
          "tree_id": "006227324f804060f35ecb04113c93f021518fc9",
          "url": "https://github.com/sgatev/lucid/commit/4a16f2a89e8641b510f7548462dfbffcb5ba048b"
        },
        "date": 1790225121276,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 46285.664,
            "unit": "ns/iter",
            "extra": "2721 iterations, 38.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 213140.208,
            "unit": "ns/iter",
            "extra": "674 iterations, 35.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 400522.472,
            "unit": "ns/iter",
            "extra": "343 iterations, 38.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 145224.751,
            "unit": "ns/iter",
            "extra": "1005 iterations, 23.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 106845.439,
            "unit": "ns/iter",
            "extra": "1425 iterations, 71.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 216488.067,
            "unit": "ns/iter",
            "extra": "667 iterations, 71.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 59544.527,
            "unit": "ns/iter",
            "extra": "2389 iterations, 56.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 371817.383,
            "unit": "ns/iter",
            "extra": "384 iterations, 20.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 900437.5,
            "unit": "ns/iter",
            "extra": "200 iterations, 3.8MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 615.044,
            "unit": "ns/iter",
            "extra": "334764 iterations, 86.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 14635.509,
            "unit": "ns/iter",
            "extra": "9278 iterations, 521.7MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 28091.367,
            "unit": "ns/iter",
            "extra": "5124 iterations, 554.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 77112.106,
            "unit": "ns/iter",
            "extra": "1884 iterations, 70.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 167479.256,
            "unit": "ns/iter",
            "extra": "924 iterations, 64.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 7926.335,
            "unit": "ns/iter",
            "extra": "20000 iterations, 426.7MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 44.963,
            "unit": "ns/iter",
            "extra": "6278015 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 20.207,
            "unit": "ns/iter",
            "extra": "6032500 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 60.109,
            "unit": "ns/iter",
            "extra": "11155600 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 25.806,
            "unit": "ns/iter",
            "extra": "5181786 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 37.409,
            "unit": "ns/iter",
            "extra": "10323246 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 32.154,
            "unit": "ns/iter",
            "extra": "4273384 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 16.175,
            "unit": "ns/iter",
            "extra": "9894896 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 5.468,
            "unit": "ns/iter",
            "extra": "27384313 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 1.816,
            "unit": "ns/iter",
            "extra": "79817559 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 7.52,
            "unit": "ns/iter",
            "extra": "13768573 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 37.083,
            "unit": "ns/iter",
            "extra": "8290585 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 22.257,
            "unit": "ns/iter",
            "extra": "5805093 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 15.861,
            "unit": "ns/iter",
            "extra": "7868152 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 6.97,
            "unit": "ns/iter",
            "extra": "23295640 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 2.016,
            "unit": "ns/iter",
            "extra": "80883944 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 9.496,
            "unit": "ns/iter",
            "extra": "15624928 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 2.695,
            "unit": "ns/iter",
            "extra": "200000000 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.474,
            "unit": "ns/iter",
            "extra": "57480112 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 6.408,
            "unit": "ns/iter",
            "extra": "22788160 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 638930.493,
            "unit": "ns/iter",
            "extra": "223 iterations, 1641.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 675368.995,
            "unit": "ns/iter",
            "extra": "222 iterations, 1552.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 676049.605,
            "unit": "ns/iter",
            "extra": "210 iterations, 1550.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 630923.194,
            "unit": "ns/iter",
            "extra": "217 iterations, 1661.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 607628,
            "unit": "ns/iter",
            "extra": "236 iterations, 1725.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 638813.454,
            "unit": "ns/iter",
            "extra": "218 iterations, 1641.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 673204.657,
            "unit": "ns/iter",
            "extra": "204 iterations, 1557.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 752226.455,
            "unit": "ns/iter",
            "extra": "200 iterations, 1385.8MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1986087.719,
            "unit": "ns/iter",
            "extra": "57 iterations, 528.0MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 2155568.548,
            "unit": "ns/iter",
            "extra": "62 iterations, 486.4MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 3322532.051,
            "unit": "ns/iter",
            "extra": "39 iterations, 315.6MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 4098815.094,
            "unit": "ns/iter",
            "extra": "32 iterations, 254.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 125557.055,
            "unit": "ns/iter",
            "extra": "1473 iterations, 124.0MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 232015.815,
            "unit": "ns/iter",
            "extra": "764 iterations, 135.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 287677.231,
            "unit": "ns/iter",
            "extra": "568 iterations, 37.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 564389.832,
            "unit": "ns/iter",
            "extra": "250 iterations, 37.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 687400.858,
            "unit": "ns/iter",
            "extra": "253 iterations, 42.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 98268.306,
            "unit": "ns/iter",
            "extra": "1550 iterations, 55.0MB/s"
          }
        ]
      },
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
          "id": "14264dbfbad1eafe274337ea8508d5af17196a2d",
          "message": "Report the fastest of three benchmark runs",
          "timestamp": "2026-09-25T22:41:07+02:00",
          "tree_id": "524e26f51c7b354ede5ab34b6e722fd45bf1b2f8",
          "url": "https://github.com/sgatev/lucid/commit/14264dbfbad1eafe274337ea8508d5af17196a2d"
        },
        "date": 1790369416597,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 50974.632,
            "unit": "ns/iter",
            "extra": "2825 iterations, 35.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 203139.877,
            "unit": "ns/iter",
            "extra": "689 iterations, 37.6MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 417475.777,
            "unit": "ns/iter",
            "extra": "332 iterations, 37.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 146765.763,
            "unit": "ns/iter",
            "extra": "949 iterations, 23.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 121887.459,
            "unit": "ns/iter",
            "extra": "1020 iterations, 62.6MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 259327.84,
            "unit": "ns/iter",
            "extra": "531 iterations, 60.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 58775.963,
            "unit": "ns/iter",
            "extra": "1921 iterations, 57.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 362535.128,
            "unit": "ns/iter",
            "extra": "376 iterations, 21.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 575143.2,
            "unit": "ns/iter",
            "extra": "245 iterations, 5.9MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 573.356,
            "unit": "ns/iter",
            "extra": "194557 iterations, 92.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 14566.392,
            "unit": "ns/iter",
            "extra": "9710 iterations, 524.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 27936.564,
            "unit": "ns/iter",
            "extra": "5051 iterations, 557.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 76331.525,
            "unit": "ns/iter",
            "extra": "1820 iterations, 71.0MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 153062.772,
            "unit": "ns/iter",
            "extra": "918 iterations, 70.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 7881.304,
            "unit": "ns/iter",
            "extra": "20000 iterations, 429.1MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 35.765,
            "unit": "ns/iter",
            "extra": "8695899 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 20.603,
            "unit": "ns/iter",
            "extra": "7335555 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 35.956,
            "unit": "ns/iter",
            "extra": "9240797 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 20.335,
            "unit": "ns/iter",
            "extra": "6087496 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 47.807,
            "unit": "ns/iter",
            "extra": "9956676 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 22.834,
            "unit": "ns/iter",
            "extra": "5824808 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 16.31,
            "unit": "ns/iter",
            "extra": "8843850 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 5.806,
            "unit": "ns/iter",
            "extra": "17757952 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 1.762,
            "unit": "ns/iter",
            "extra": "79303264 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 6.221,
            "unit": "ns/iter",
            "extra": "20100260 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 30.794,
            "unit": "ns/iter",
            "extra": "10408793 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 10.771,
            "unit": "ns/iter",
            "extra": "12071437 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 7.792,
            "unit": "ns/iter",
            "extra": "18889244 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 4.278,
            "unit": "ns/iter",
            "extra": "31873417 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 1.711,
            "unit": "ns/iter",
            "extra": "80304008 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 5.037,
            "unit": "ns/iter",
            "extra": "28358979 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.523,
            "unit": "ns/iter",
            "extra": "200000000 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.357,
            "unit": "ns/iter",
            "extra": "58774139 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 6.005,
            "unit": "ns/iter",
            "extra": "24675403 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 717846.04,
            "unit": "ns/iter",
            "extra": "200 iterations, 1460.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 745613.125,
            "unit": "ns/iter",
            "extra": "200 iterations, 1406.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 731620,
            "unit": "ns/iter",
            "extra": "200 iterations, 1433.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 682102.29,
            "unit": "ns/iter",
            "extra": "200 iterations, 1537.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 640789.149,
            "unit": "ns/iter",
            "extra": "215 iterations, 1636.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 626602.841,
            "unit": "ns/iter",
            "extra": "220 iterations, 1673.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 652209.951,
            "unit": "ns/iter",
            "extra": "206 iterations, 1607.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 720610.42,
            "unit": "ns/iter",
            "extra": "200 iterations, 1446.6MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1967080.683,
            "unit": "ns/iter",
            "extra": "63 iterations, 533.1MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1993376.186,
            "unit": "ns/iter",
            "extra": "70 iterations, 526.0MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 3054091.262,
            "unit": "ns/iter",
            "extra": "42 iterations, 343.3MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 3756494.029,
            "unit": "ns/iter",
            "extra": "35 iterations, 277.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 95817.317,
            "unit": "ns/iter",
            "extra": "1436 iterations, 162.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 226562.699,
            "unit": "ns/iter",
            "extra": "836 iterations, 139.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 235565.841,
            "unit": "ns/iter",
            "extra": "605 iterations, 45.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 462930.603,
            "unit": "ns/iter",
            "extra": "287 iterations, 46.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 538775.085,
            "unit": "ns/iter",
            "extra": "294 iterations, 53.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 114587.667,
            "unit": "ns/iter",
            "extra": "1875 iterations, 47.2MB/s"
          }
        ]
      },
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
          "id": "890d290b2f099fa749f7390ed2c97d2c849db62a",
          "message": "Set read permission in jobs.test CI action",
          "timestamp": "2026-09-25T23:07:38+02:00",
          "tree_id": "f923ecbb5fe7d04bb0ac383417b88871ffbd3ae2",
          "url": "https://github.com/sgatev/lucid/commit/890d290b2f099fa749f7390ed2c97d2c849db62a"
        },
        "date": 1790370874116,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 49599.292,
            "unit": "ns/iter",
            "extra": "2799 iterations, 36.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 204848.298,
            "unit": "ns/iter",
            "extra": "685 iterations, 37.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 389424.734,
            "unit": "ns/iter",
            "extra": "346 iterations, 40.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 144547.853,
            "unit": "ns/iter",
            "extra": "943 iterations, 23.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 103946.598,
            "unit": "ns/iter",
            "extra": "1335 iterations, 73.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 209478.817,
            "unit": "ns/iter",
            "extra": "655 iterations, 74.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 57853.605,
            "unit": "ns/iter",
            "extra": "2450 iterations, 58.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 350322.253,
            "unit": "ns/iter",
            "extra": "376 iterations, 21.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 568066.669,
            "unit": "ns/iter",
            "extra": "245 iterations, 6.0MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 622.778,
            "unit": "ns/iter",
            "extra": "156243 iterations, 85.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 14882.501,
            "unit": "ns/iter",
            "extra": "9338 iterations, 513.0MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 27240.708,
            "unit": "ns/iter",
            "extra": "5036 iterations, 571.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 77109.088,
            "unit": "ns/iter",
            "extra": "1854 iterations, 70.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 154839.198,
            "unit": "ns/iter",
            "extra": "881 iterations, 69.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 7820.94,
            "unit": "ns/iter",
            "extra": "20000 iterations, 432.4MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 36.947,
            "unit": "ns/iter",
            "extra": "9683583 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 17.057,
            "unit": "ns/iter",
            "extra": "7675333 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 33.982,
            "unit": "ns/iter",
            "extra": "10798129 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 18.729,
            "unit": "ns/iter",
            "extra": "6666904 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 35.123,
            "unit": "ns/iter",
            "extra": "11243701 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 18.876,
            "unit": "ns/iter",
            "extra": "5856382 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 17.673,
            "unit": "ns/iter",
            "extra": "8102594 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 5.627,
            "unit": "ns/iter",
            "extra": "23725632 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 1.795,
            "unit": "ns/iter",
            "extra": "79492790 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 5.363,
            "unit": "ns/iter",
            "extra": "20895132 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 23.026,
            "unit": "ns/iter",
            "extra": "4713229 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 6.492,
            "unit": "ns/iter",
            "extra": "15037458 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 5.373,
            "unit": "ns/iter",
            "extra": "19969452 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 5.128,
            "unit": "ns/iter",
            "extra": "30248739 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 1.585,
            "unit": "ns/iter",
            "extra": "82318609 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 4.134,
            "unit": "ns/iter",
            "extra": "31569749 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.497,
            "unit": "ns/iter",
            "extra": "243785112 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.174,
            "unit": "ns/iter",
            "extra": "64344385 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 5.733,
            "unit": "ns/iter",
            "extra": "25123937 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 599258.797,
            "unit": "ns/iter",
            "extra": "232 iterations, 1749.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 600979.164,
            "unit": "ns/iter",
            "extra": "232 iterations, 1744.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 661708.138,
            "unit": "ns/iter",
            "extra": "210 iterations, 1584.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 630954.969,
            "unit": "ns/iter",
            "extra": "223 iterations, 1661.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 582785.891,
            "unit": "ns/iter",
            "extra": "238 iterations, 1799.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 590325.793,
            "unit": "ns/iter",
            "extra": "232 iterations, 1776.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 651685.556,
            "unit": "ns/iter",
            "extra": "214 iterations, 1608.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 666882.29,
            "unit": "ns/iter",
            "extra": "200 iterations, 1563.1MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1826144.928,
            "unit": "ns/iter",
            "extra": "69 iterations, 574.2MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1870568.493,
            "unit": "ns/iter",
            "extra": "73 iterations, 560.6MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2889701.867,
            "unit": "ns/iter",
            "extra": "45 iterations, 362.8MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 3570565.324,
            "unit": "ns/iter",
            "extra": "37 iterations, 291.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 84161.67,
            "unit": "ns/iter",
            "extra": "1551 iterations, 185.0MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 160837.953,
            "unit": "ns/iter",
            "extra": "1001 iterations, 195.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 211975.741,
            "unit": "ns/iter",
            "extra": "663 iterations, 50.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 434224.561,
            "unit": "ns/iter",
            "extra": "321 iterations, 49.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 496514.096,
            "unit": "ns/iter",
            "extra": "272 iterations, 58.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 89105.05,
            "unit": "ns/iter",
            "extra": "1959 iterations, 60.7MB/s"
          }
        ]
      },
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
          "id": "d359932083b806e8f291d31a7c4e66288954018e",
          "message": "Report the names that are not there",
          "timestamp": "2026-09-25T23:47:49+02:00",
          "tree_id": "b54a4cdba763964b99a1b789f66fffe289f3d5a9",
          "url": "https://github.com/sgatev/lucid/commit/d359932083b806e8f291d31a7c4e66288954018e"
        },
        "date": 1790373256162,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 50890.807,
            "unit": "ns/iter",
            "extra": "3664 iterations, 35.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 205689.668,
            "unit": "ns/iter",
            "extra": "692 iterations, 37.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 414391.559,
            "unit": "ns/iter",
            "extra": "229 iterations, 37.6MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 145312.541,
            "unit": "ns/iter",
            "extra": "992 iterations, 23.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 103384.221,
            "unit": "ns/iter",
            "extra": "1324 iterations, 73.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 211061.627,
            "unit": "ns/iter",
            "extra": "668 iterations, 73.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 59112.367,
            "unit": "ns/iter",
            "extra": "2411 iterations, 57.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 368910.401,
            "unit": "ns/iter",
            "extra": "379 iterations, 20.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 551234.568,
            "unit": "ns/iter",
            "extra": "243 iterations, 6.1MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 581.297,
            "unit": "ns/iter",
            "extra": "186278 iterations, 91.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 14316.728,
            "unit": "ns/iter",
            "extra": "9856 iterations, 533.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 27391.442,
            "unit": "ns/iter",
            "extra": "5228 iterations, 568.5MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 74581.363,
            "unit": "ns/iter",
            "extra": "1903 iterations, 72.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 149788.792,
            "unit": "ns/iter",
            "extra": "913 iterations, 71.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 7819.231,
            "unit": "ns/iter",
            "extra": "20000 iterations, 432.5MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 28.857,
            "unit": "ns/iter",
            "extra": "7889379 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 21.844,
            "unit": "ns/iter",
            "extra": "8320394 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 33.62,
            "unit": "ns/iter",
            "extra": "10057501 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 18.629,
            "unit": "ns/iter",
            "extra": "7786375 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 32.926,
            "unit": "ns/iter",
            "extra": "10168045 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 20.747,
            "unit": "ns/iter",
            "extra": "6246723 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 13.808,
            "unit": "ns/iter",
            "extra": "8067982 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 4.662,
            "unit": "ns/iter",
            "extra": "26688480 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 1.813,
            "unit": "ns/iter",
            "extra": "74846297 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 6.755,
            "unit": "ns/iter",
            "extra": "25349018 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 29.869,
            "unit": "ns/iter",
            "extra": "9217727 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 6.449,
            "unit": "ns/iter",
            "extra": "24420203 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 4.183,
            "unit": "ns/iter",
            "extra": "34039788 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 3.449,
            "unit": "ns/iter",
            "extra": "41936043 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 1.592,
            "unit": "ns/iter",
            "extra": "88621617 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 4.216,
            "unit": "ns/iter",
            "extra": "24679571 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.498,
            "unit": "ns/iter",
            "extra": "251031771 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.178,
            "unit": "ns/iter",
            "extra": "62861312 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 5.593,
            "unit": "ns/iter",
            "extra": "26740945 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 663036.661,
            "unit": "ns/iter",
            "extra": "233 iterations, 1581.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 603444.034,
            "unit": "ns/iter",
            "extra": "204 iterations, 1737.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 665644.448,
            "unit": "ns/iter",
            "extra": "210 iterations, 1575.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 628064.274,
            "unit": "ns/iter",
            "extra": "223 iterations, 1669.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 636743.545,
            "unit": "ns/iter",
            "extra": "213 iterations, 1646.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 568449.813,
            "unit": "ns/iter",
            "extra": "225 iterations, 1844.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 597837.073,
            "unit": "ns/iter",
            "extra": "234 iterations, 1753.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 664949.8,
            "unit": "ns/iter",
            "extra": "210 iterations, 1567.7MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 2029856.577,
            "unit": "ns/iter",
            "extra": "52 iterations, 516.6MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1913285.714,
            "unit": "ns/iter",
            "extra": "70 iterations, 548.0MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 3048486.419,
            "unit": "ns/iter",
            "extra": "43 iterations, 343.9MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 3684812.5,
            "unit": "ns/iter",
            "extra": "36 iterations, 282.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 85673.543,
            "unit": "ns/iter",
            "extra": "1727 iterations, 181.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 161911.91,
            "unit": "ns/iter",
            "extra": "946 iterations, 194.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 233597.052,
            "unit": "ns/iter",
            "extra": "653 iterations, 45.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 443800.191,
            "unit": "ns/iter",
            "extra": "303 iterations, 48.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 482974.418,
            "unit": "ns/iter",
            "extra": "316 iterations, 60.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 82654.71,
            "unit": "ns/iter",
            "extra": "1955 iterations, 65.4MB/s"
          }
        ]
      },
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
          "id": "04564f2edf2ac2b60d0257f96c1ffcd89553980f",
          "message": "Combine conditions with and, or and not",
          "timestamp": "2026-09-26T20:00:58+02:00",
          "tree_id": "199f873c3ebb669ba0a505461dbd5b5bac83ba41",
          "url": "https://github.com/sgatev/lucid/commit/04564f2edf2ac2b60d0257f96c1ffcd89553980f"
        },
        "date": 1790446089701,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 51131.48,
            "unit": "ns/iter",
            "extra": "2797 iterations, 35.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 204425.551,
            "unit": "ns/iter",
            "extra": "666 iterations, 37.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 401462.066,
            "unit": "ns/iter",
            "extra": "346 iterations, 38.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 148457.777,
            "unit": "ns/iter",
            "extra": "974 iterations, 22.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 105851.095,
            "unit": "ns/iter",
            "extra": "1248 iterations, 72.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 210205.249,
            "unit": "ns/iter",
            "extra": "662 iterations, 74.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 57902.212,
            "unit": "ns/iter",
            "extra": "2381 iterations, 58.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 361102.018,
            "unit": "ns/iter",
            "extra": "388 iterations, 21.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 559480.392,
            "unit": "ns/iter",
            "extra": "255 iterations, 6.0MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 598.593,
            "unit": "ns/iter",
            "extra": "179813 iterations, 88.5MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 14429.95,
            "unit": "ns/iter",
            "extra": "9514 iterations, 529.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 27456.001,
            "unit": "ns/iter",
            "extra": "5162 iterations, 567.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 86090.236,
            "unit": "ns/iter",
            "extra": "1666 iterations, 62.9MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 181075.473,
            "unit": "ns/iter",
            "extra": "615 iterations, 59.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 7993.585,
            "unit": "ns/iter",
            "extra": "20000 iterations, 423.1MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 31.022,
            "unit": "ns/iter",
            "extra": "7221132 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 20.731,
            "unit": "ns/iter",
            "extra": "7173585 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 39.103,
            "unit": "ns/iter",
            "extra": "9523809 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 20.998,
            "unit": "ns/iter",
            "extra": "6725945 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 36.081,
            "unit": "ns/iter",
            "extra": "9432426 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 22.821,
            "unit": "ns/iter",
            "extra": "5884897 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 15.234,
            "unit": "ns/iter",
            "extra": "10003363 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 5.206,
            "unit": "ns/iter",
            "extra": "26638972 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 1.71,
            "unit": "ns/iter",
            "extra": "80484840 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 4.708,
            "unit": "ns/iter",
            "extra": "22304536 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 29.487,
            "unit": "ns/iter",
            "extra": "10053107 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 7.295,
            "unit": "ns/iter",
            "extra": "17444758 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 4.16,
            "unit": "ns/iter",
            "extra": "33857315 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 3.386,
            "unit": "ns/iter",
            "extra": "41993176 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 1.578,
            "unit": "ns/iter",
            "extra": "88311806 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 4.091,
            "unit": "ns/iter",
            "extra": "32385547 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.495,
            "unit": "ns/iter",
            "extra": "200000000 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.178,
            "unit": "ns/iter",
            "extra": "63736545 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 5.641,
            "unit": "ns/iter",
            "extra": "23958416 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 652619.048,
            "unit": "ns/iter",
            "extra": "210 iterations, 1606.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 619182.631,
            "unit": "ns/iter",
            "extra": "214 iterations, 1693.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 658456.46,
            "unit": "ns/iter",
            "extra": "200 iterations, 1592.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 624433.964,
            "unit": "ns/iter",
            "extra": "224 iterations, 1679.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 582113.021,
            "unit": "ns/iter",
            "extra": "240 iterations, 1801.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 566656.377,
            "unit": "ns/iter",
            "extra": "247 iterations, 1850.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 597650.575,
            "unit": "ns/iter",
            "extra": "233 iterations, 1754.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 665523.214,
            "unit": "ns/iter",
            "extra": "210 iterations, 1566.3MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1870853.261,
            "unit": "ns/iter",
            "extra": "69 iterations, 560.5MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1923644.958,
            "unit": "ns/iter",
            "extra": "71 iterations, 545.1MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 3035943.182,
            "unit": "ns/iter",
            "extra": "44 iterations, 345.4MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 3749069.057,
            "unit": "ns/iter",
            "extra": "35 iterations, 278.0MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 82380.077,
            "unit": "ns/iter",
            "extra": "1789 iterations, 189.0MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 163913.144,
            "unit": "ns/iter",
            "extra": "970 iterations, 192.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 218726.073,
            "unit": "ns/iter",
            "extra": "660 iterations, 49.0MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 445694.708,
            "unit": "ns/iter",
            "extra": "315 iterations, 48.0MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 493105.735,
            "unit": "ns/iter",
            "extra": "279 iterations, 58.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 84915.712,
            "unit": "ns/iter",
            "extra": "2052 iterations, 63.7MB/s"
          }
        ]
      },
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
          "id": "7e8d23a46a5fc2c60655e4a1de81a8b278942c3c",
          "message": "Work out a string during compilation",
          "timestamp": "2026-09-26T23:07:00+02:00",
          "tree_id": "31fca27215962d982cfac36fa2756880916f48ba",
          "url": "https://github.com/sgatev/lucid/commit/7e8d23a46a5fc2c60655e4a1de81a8b278942c3c"
        },
        "date": 1790457488242,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 65374.907,
            "unit": "ns/iter",
            "extra": "2678 iterations, 27.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 300772.486,
            "unit": "ns/iter",
            "extra": "630 iterations, 25.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 575146.875,
            "unit": "ns/iter",
            "extra": "200 iterations, 27.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 196216.364,
            "unit": "ns/iter",
            "extra": "742 iterations, 17.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 127639.805,
            "unit": "ns/iter",
            "extra": "1092 iterations, 59.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 298781.85,
            "unit": "ns/iter",
            "extra": "505 iterations, 52.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 73442.686,
            "unit": "ns/iter",
            "extra": "1848 iterations, 46.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 455560.438,
            "unit": "ns/iter",
            "extra": "313 iterations, 16.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 619765.498,
            "unit": "ns/iter",
            "extra": "207 iterations, 5.5MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 789.34,
            "unit": "ns/iter",
            "extra": "197406 iterations, 67.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 21582.969,
            "unit": "ns/iter",
            "extra": "8917 iterations, 353.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 39238.687,
            "unit": "ns/iter",
            "extra": "4707 iterations, 396.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 119099.077,
            "unit": "ns/iter",
            "extra": "1760 iterations, 45.5MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 203130.501,
            "unit": "ns/iter",
            "extra": "659 iterations, 52.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 11483.025,
            "unit": "ns/iter",
            "extra": "10000 iterations, 294.5MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 50.68,
            "unit": "ns/iter",
            "extra": "7842569 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 44.656,
            "unit": "ns/iter",
            "extra": "5419485 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 48.924,
            "unit": "ns/iter",
            "extra": "4895276 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 40.667,
            "unit": "ns/iter",
            "extra": "4165700 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 44.419,
            "unit": "ns/iter",
            "extra": "4911670 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 37.875,
            "unit": "ns/iter",
            "extra": "3228106 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 16.765,
            "unit": "ns/iter",
            "extra": "4654926 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 7.007,
            "unit": "ns/iter",
            "extra": "21045574 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 2.282,
            "unit": "ns/iter",
            "extra": "55322301 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 8.694,
            "unit": "ns/iter",
            "extra": "17237220 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 37.862,
            "unit": "ns/iter",
            "extra": "6558810 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 18.736,
            "unit": "ns/iter",
            "extra": "6899602 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 16.668,
            "unit": "ns/iter",
            "extra": "10973542 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 6.976,
            "unit": "ns/iter",
            "extra": "9155587 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 2.634,
            "unit": "ns/iter",
            "extra": "75481870 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 8.939,
            "unit": "ns/iter",
            "extra": "16696314 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.755,
            "unit": "ns/iter",
            "extra": "85677112 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 3.168,
            "unit": "ns/iter",
            "extra": "54152490 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 6.661,
            "unit": "ns/iter",
            "extra": "24484085 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 765926.04,
            "unit": "ns/iter",
            "extra": "200 iterations, 1369.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 813585.286,
            "unit": "ns/iter",
            "extra": "192 iterations, 1288.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 952312.5,
            "unit": "ns/iter",
            "extra": "100 iterations, 1100.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 827199.585,
            "unit": "ns/iter",
            "extra": "200 iterations, 1267.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 704625.219,
            "unit": "ns/iter",
            "extra": "192 iterations, 1488.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 713526.875,
            "unit": "ns/iter",
            "extra": "200 iterations, 1469.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 723648.955,
            "unit": "ns/iter",
            "extra": "200 iterations, 1448.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 796464.79,
            "unit": "ns/iter",
            "extra": "200 iterations, 1308.8MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 2902211.914,
            "unit": "ns/iter",
            "extra": "35 iterations, 361.3MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 2401437.153,
            "unit": "ns/iter",
            "extra": "59 iterations, 436.6MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 4621089.286,
            "unit": "ns/iter",
            "extra": "35 iterations, 226.9MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 6108908.32,
            "unit": "ns/iter",
            "extra": "25 iterations, 170.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 218693.012,
            "unit": "ns/iter",
            "extra": "824 iterations, 71.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 220924.726,
            "unit": "ns/iter",
            "extra": "667 iterations, 142.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 259604.747,
            "unit": "ns/iter",
            "extra": "502 iterations, 41.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 605648.859,
            "unit": "ns/iter",
            "extra": "234 iterations, 35.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 530189.362,
            "unit": "ns/iter",
            "extra": "224 iterations, 54.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 121409.084,
            "unit": "ns/iter",
            "extra": "1544 iterations, 44.5MB/s"
          }
        ]
      },
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
          "id": "1cf2ff2dd2dfb35a2ccce616bee258aacb292d0b",
          "message": "Print the graph a stage at a time",
          "timestamp": "2026-09-27T13:52:55+02:00",
          "tree_id": "16a6a0b62b460b6f3edf17f9ccb7d627392313a7",
          "url": "https://github.com/sgatev/lucid/commit/1cf2ff2dd2dfb35a2ccce616bee258aacb292d0b"
        },
        "date": 1790510500555,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 10223.579,
            "unit": "ns/iter",
            "extra": "10000 iterations, 175.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 38223.828,
            "unit": "ns/iter",
            "extra": "3528 iterations, 199.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 76651.231,
            "unit": "ns/iter",
            "extra": "1841 iterations, 203.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 26483.864,
            "unit": "ns/iter",
            "extra": "5167 iterations, 127.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 33187.583,
            "unit": "ns/iter",
            "extra": "4283 iterations, 230.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 65022.634,
            "unit": "ns/iter",
            "extra": "2163 iterations, 239.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 28891.641,
            "unit": "ns/iter",
            "extra": "4552 iterations, 117.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 139108.545,
            "unit": "ns/iter",
            "extra": "1028 iterations, 54.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 401597.934,
            "unit": "ns/iter",
            "extra": "331 iterations, 8.4MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 327.393,
            "unit": "ns/iter",
            "extra": "327501 iterations, 161.9MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 14531.215,
            "unit": "ns/iter",
            "extra": "9600 iterations, 525.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 27569.74,
            "unit": "ns/iter",
            "extra": "5162 iterations, 564.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 73094.519,
            "unit": "ns/iter",
            "extra": "1937 iterations, 74.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 146544.834,
            "unit": "ns/iter",
            "extra": "934 iterations, 73.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 7831.152,
            "unit": "ns/iter",
            "extra": "20000 iterations, 431.9MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 35.316,
            "unit": "ns/iter",
            "extra": "8541489 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 20.226,
            "unit": "ns/iter",
            "extra": "6119170 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 35.265,
            "unit": "ns/iter",
            "extra": "10742134 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 20.919,
            "unit": "ns/iter",
            "extra": "6077652 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 37.461,
            "unit": "ns/iter",
            "extra": "10152928 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 20.185,
            "unit": "ns/iter",
            "extra": "5183601 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 19.797,
            "unit": "ns/iter",
            "extra": "8274068 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 6.187,
            "unit": "ns/iter",
            "extra": "19003556 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 7.477,
            "unit": "ns/iter",
            "extra": "18013091 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 29.753,
            "unit": "ns/iter",
            "extra": "7713056 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 20.521,
            "unit": "ns/iter",
            "extra": "6662608 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 7.624,
            "unit": "ns/iter",
            "extra": "16017619 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 4.923,
            "unit": "ns/iter",
            "extra": "30016077 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 5.843,
            "unit": "ns/iter",
            "extra": "20428761 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.508,
            "unit": "ns/iter",
            "extra": "243929709 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.186,
            "unit": "ns/iter",
            "extra": "62180768 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 5.591,
            "unit": "ns/iter",
            "extra": "26903246 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 599866.37,
            "unit": "ns/iter",
            "extra": "227 iterations, 1747.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 610703.912,
            "unit": "ns/iter",
            "extra": "226 iterations, 1716.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 666677.033,
            "unit": "ns/iter",
            "extra": "209 iterations, 1572.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 729008.75,
            "unit": "ns/iter",
            "extra": "200 iterations, 1438.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 588846.771,
            "unit": "ns/iter",
            "extra": "214 iterations, 1780.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 570398.469,
            "unit": "ns/iter",
            "extra": "245 iterations, 1838.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 599935.573,
            "unit": "ns/iter",
            "extra": "227 iterations, 1747.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 671329.063,
            "unit": "ns/iter",
            "extra": "205 iterations, 1552.8MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1761120.958,
            "unit": "ns/iter",
            "extra": "72 iterations, 595.4MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1804590,
            "unit": "ns/iter",
            "extra": "75 iterations, 581.1MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2865009.267,
            "unit": "ns/iter",
            "extra": "45 iterations, 366.0MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 3597432.432,
            "unit": "ns/iter",
            "extra": "37 iterations, 289.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 82338.474,
            "unit": "ns/iter",
            "extra": "1678 iterations, 189.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 169651.042,
            "unit": "ns/iter",
            "extra": "928 iterations, 185.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 258347.56,
            "unit": "ns/iter",
            "extra": "618 iterations, 41.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 433024.306,
            "unit": "ns/iter",
            "extra": "288 iterations, 49.4MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 472596.774,
            "unit": "ns/iter",
            "extra": "310 iterations, 61.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 81349.838,
            "unit": "ns/iter",
            "extra": "2007 iterations, 66.4MB/s"
          }
        ]
      },
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
          "id": "f4787e7f3bda73cb4ce15645599300e8e1538ac7",
          "message": "Add header images to README",
          "timestamp": "2026-09-27T14:19:45+02:00",
          "tree_id": "5661edd63daa1a0ed5757b3a4cf7f2d6ea7f790f",
          "url": "https://github.com/sgatev/lucid/commit/f4787e7f3bda73cb4ce15645599300e8e1538ac7"
        },
        "date": 1790512001187,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 11062.533,
            "unit": "ns/iter",
            "extra": "10000 iterations, 161.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 40487.261,
            "unit": "ns/iter",
            "extra": "3094 iterations, 188.6MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 85340.132,
            "unit": "ns/iter",
            "extra": "1765 iterations, 182.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 31973.971,
            "unit": "ns/iter",
            "extra": "4732 iterations, 105.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 36143.059,
            "unit": "ns/iter",
            "extra": "3454 iterations, 211.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 69904.474,
            "unit": "ns/iter",
            "extra": "1948 iterations, 222.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 30807.879,
            "unit": "ns/iter",
            "extra": "4526 iterations, 109.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 133787.356,
            "unit": "ns/iter",
            "extra": "1073 iterations, 57.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 377752.226,
            "unit": "ns/iter",
            "extra": "337 iterations, 9.0MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 364.83,
            "unit": "ns/iter",
            "extra": "514216 iterations, 145.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 15876.417,
            "unit": "ns/iter",
            "extra": "9586 iterations, 480.9MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 29033.586,
            "unit": "ns/iter",
            "extra": "4940 iterations, 536.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 77775.422,
            "unit": "ns/iter",
            "extra": "1816 iterations, 69.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 160092.773,
            "unit": "ns/iter",
            "extra": "896 iterations, 67.0MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 9330.821,
            "unit": "ns/iter",
            "extra": "10000 iterations, 362.5MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 33.111,
            "unit": "ns/iter",
            "extra": "6178844 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 29.216,
            "unit": "ns/iter",
            "extra": "6112702 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 33.402,
            "unit": "ns/iter",
            "extra": "7346815 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 25.72,
            "unit": "ns/iter",
            "extra": "6699940 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 43.36,
            "unit": "ns/iter",
            "extra": "9200791 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 28.381,
            "unit": "ns/iter",
            "extra": "4938264 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 17.97,
            "unit": "ns/iter",
            "extra": "7538601 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 6.731,
            "unit": "ns/iter",
            "extra": "21646276 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 8.464,
            "unit": "ns/iter",
            "extra": "16528355 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 28.79,
            "unit": "ns/iter",
            "extra": "5239994 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 21.425,
            "unit": "ns/iter",
            "extra": "5344650 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 12.752,
            "unit": "ns/iter",
            "extra": "9211637 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 5.508,
            "unit": "ns/iter",
            "extra": "27482186 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 6.483,
            "unit": "ns/iter",
            "extra": "20687489 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.534,
            "unit": "ns/iter",
            "extra": "200000000 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.34,
            "unit": "ns/iter",
            "extra": "59559692 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 5.956,
            "unit": "ns/iter",
            "extra": "24787715 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 643952.189,
            "unit": "ns/iter",
            "extra": "217 iterations, 1628.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 650500.389,
            "unit": "ns/iter",
            "extra": "216 iterations, 1611.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 714917.71,
            "unit": "ns/iter",
            "extra": "200 iterations, 1466.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 670940.738,
            "unit": "ns/iter",
            "extra": "206 iterations, 1562.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 687020.74,
            "unit": "ns/iter",
            "extra": "223 iterations, 1526.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 656257.109,
            "unit": "ns/iter",
            "extra": "211 iterations, 1597.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 643409.562,
            "unit": "ns/iter",
            "extra": "217 iterations, 1629.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 716583.125,
            "unit": "ns/iter",
            "extra": "200 iterations, 1454.7MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1900661.687,
            "unit": "ns/iter",
            "extra": "67 iterations, 551.7MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1942119.643,
            "unit": "ns/iter",
            "extra": "70 iterations, 539.9MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 3030675.911,
            "unit": "ns/iter",
            "extra": "45 iterations, 346.0MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 3809997.694,
            "unit": "ns/iter",
            "extra": "36 iterations, 273.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 107027.978,
            "unit": "ns/iter",
            "extra": "1735 iterations, 145.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 193610.31,
            "unit": "ns/iter",
            "extra": "780 iterations, 162.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 231367.75,
            "unit": "ns/iter",
            "extra": "569 iterations, 46.4MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 594919.679,
            "unit": "ns/iter",
            "extra": "249 iterations, 35.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 532293.274,
            "unit": "ns/iter",
            "extra": "259 iterations, 54.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 95451.575,
            "unit": "ns/iter",
            "extra": "1794 iterations, 56.6MB/s"
          }
        ]
      },
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
          "id": "7c57c497ee0d7a38d6bd8ef3b02e11e0fea9be89",
          "message": "Add header images to README",
          "timestamp": "2026-09-27T14:20:45+02:00",
          "tree_id": "6d2511a429399639ff3238691cefc7661c94da1c",
          "url": "https://github.com/sgatev/lucid/commit/7c57c497ee0d7a38d6bd8ef3b02e11e0fea9be89"
        },
        "date": 1790512110341,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 10058.042,
            "unit": "ns/iter",
            "extra": "10000 iterations, 178.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 38213.245,
            "unit": "ns/iter",
            "extra": "3614 iterations, 199.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 75953.498,
            "unit": "ns/iter",
            "extra": "1844 iterations, 205.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 27614.382,
            "unit": "ns/iter",
            "extra": "5027 iterations, 122.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 32446.327,
            "unit": "ns/iter",
            "extra": "4154 iterations, 235.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 64907.835,
            "unit": "ns/iter",
            "extra": "2425 iterations, 239.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 29150.512,
            "unit": "ns/iter",
            "extra": "4725 iterations, 116.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 112785.483,
            "unit": "ns/iter",
            "extra": "1186 iterations, 67.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 356342.437,
            "unit": "ns/iter",
            "extra": "389 iterations, 9.5MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 317.604,
            "unit": "ns/iter",
            "extra": "640438 iterations, 166.9MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 14344.645,
            "unit": "ns/iter",
            "extra": "9367 iterations, 532.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 27469.614,
            "unit": "ns/iter",
            "extra": "5138 iterations, 566.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 71920.412,
            "unit": "ns/iter",
            "extra": "1936 iterations, 75.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 143644.896,
            "unit": "ns/iter",
            "extra": "978 iterations, 74.7MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 7749.969,
            "unit": "ns/iter",
            "extra": "20000 iterations, 436.4MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 48.767,
            "unit": "ns/iter",
            "extra": "9262781 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 21.661,
            "unit": "ns/iter",
            "extra": "6287213 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 34.809,
            "unit": "ns/iter",
            "extra": "10566735 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 21.708,
            "unit": "ns/iter",
            "extra": "5311681 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 35.22,
            "unit": "ns/iter",
            "extra": "10207801 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 23.6,
            "unit": "ns/iter",
            "extra": "6902871 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 17.337,
            "unit": "ns/iter",
            "extra": "9317157 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 5.815,
            "unit": "ns/iter",
            "extra": "25807640 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 6.629,
            "unit": "ns/iter",
            "extra": "17709189 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 28.274,
            "unit": "ns/iter",
            "extra": "11188289 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 9.937,
            "unit": "ns/iter",
            "extra": "13674378 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 4.843,
            "unit": "ns/iter",
            "extra": "34463302 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 4.717,
            "unit": "ns/iter",
            "extra": "42343520 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 5.069,
            "unit": "ns/iter",
            "extra": "33950019 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.5,
            "unit": "ns/iter",
            "extra": "247045743 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.182,
            "unit": "ns/iter",
            "extra": "58327257 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 5.631,
            "unit": "ns/iter",
            "extra": "27004219 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 599342.455,
            "unit": "ns/iter",
            "extra": "233 iterations, 1749.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 601808.371,
            "unit": "ns/iter",
            "extra": "232 iterations, 1742.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 688370.835,
            "unit": "ns/iter",
            "extra": "200 iterations, 1523.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 625239.768,
            "unit": "ns/iter",
            "extra": "224 iterations, 1677.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 582316.146,
            "unit": "ns/iter",
            "extra": "240 iterations, 1800.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 567117.886,
            "unit": "ns/iter",
            "extra": "246 iterations, 1848.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 597899.457,
            "unit": "ns/iter",
            "extra": "230 iterations, 1753.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 666077.899,
            "unit": "ns/iter",
            "extra": "207 iterations, 1565.0MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1760656.933,
            "unit": "ns/iter",
            "extra": "60 iterations, 595.6MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1804941.324,
            "unit": "ns/iter",
            "extra": "71 iterations, 580.9MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2835928.977,
            "unit": "ns/iter",
            "extra": "44 iterations, 369.7MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 3873204.545,
            "unit": "ns/iter",
            "extra": "33 iterations, 269.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 81528.146,
            "unit": "ns/iter",
            "extra": "1587 iterations, 191.0MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 157665.513,
            "unit": "ns/iter",
            "extra": "1011 iterations, 199.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 208086.184,
            "unit": "ns/iter",
            "extra": "614 iterations, 51.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 427085.593,
            "unit": "ns/iter",
            "extra": "332 iterations, 50.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 461789.325,
            "unit": "ns/iter",
            "extra": "320 iterations, 62.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 77578.664,
            "unit": "ns/iter",
            "extra": "1981 iterations, 69.7MB/s"
          }
        ]
      },
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
          "id": "17f5ce4f5cfbca6eb1421718d99c4c597b7213ab",
          "message": "Return a string from a random program",
          "timestamp": "2026-09-27T19:35:48+02:00",
          "tree_id": "97fbfc3ce2396bc133377c11731015748ef3df13",
          "url": "https://github.com/sgatev/lucid/commit/17f5ce4f5cfbca6eb1421718d99c4c597b7213ab"
        },
        "date": 1790531249070,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 10207.056,
            "unit": "ns/iter",
            "extra": "20000 iterations, 175.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 41889.835,
            "unit": "ns/iter",
            "extra": "3025 iterations, 182.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 75924.504,
            "unit": "ns/iter",
            "extra": "1797 iterations, 205.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 27646.238,
            "unit": "ns/iter",
            "extra": "4020 iterations, 122.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 31938.691,
            "unit": "ns/iter",
            "extra": "4391 iterations, 239.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 72008.658,
            "unit": "ns/iter",
            "extra": "2132 iterations, 216.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 27714.241,
            "unit": "ns/iter",
            "extra": "5022 iterations, 122.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 119182.872,
            "unit": "ns/iter",
            "extra": "1175 iterations, 64.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 401403.451,
            "unit": "ns/iter",
            "extra": "350 iterations, 8.4MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 306.122,
            "unit": "ns/iter",
            "extra": "625372 iterations, 173.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 14733.318,
            "unit": "ns/iter",
            "extra": "9946 iterations, 518.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 35148.885,
            "unit": "ns/iter",
            "extra": "3332 iterations, 443.0MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 75042.649,
            "unit": "ns/iter",
            "extra": "1782 iterations, 72.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 152386.037,
            "unit": "ns/iter",
            "extra": "940 iterations, 70.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 8052.602,
            "unit": "ns/iter",
            "extra": "20000 iterations, 420.0MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 28.075,
            "unit": "ns/iter",
            "extra": "8057843 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 18.27,
            "unit": "ns/iter",
            "extra": "7691304 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 33.649,
            "unit": "ns/iter",
            "extra": "11302931 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 18.338,
            "unit": "ns/iter",
            "extra": "7316404 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 34.009,
            "unit": "ns/iter",
            "extra": "10822023 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 17.286,
            "unit": "ns/iter",
            "extra": "6131677 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 15.1,
            "unit": "ns/iter",
            "extra": "10330546 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 5.829,
            "unit": "ns/iter",
            "extra": "24060840 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 7.108,
            "unit": "ns/iter",
            "extra": "19083428 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 32.699,
            "unit": "ns/iter",
            "extra": "9178246 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 11.148,
            "unit": "ns/iter",
            "extra": "10305421 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 4.002,
            "unit": "ns/iter",
            "extra": "23198492 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 4.253,
            "unit": "ns/iter",
            "extra": "42804724 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 4.85,
            "unit": "ns/iter",
            "extra": "24370785 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.498,
            "unit": "ns/iter",
            "extra": "200000000 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.168,
            "unit": "ns/iter",
            "extra": "62068182 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 5.741,
            "unit": "ns/iter",
            "extra": "29463604 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 598480.867,
            "unit": "ns/iter",
            "extra": "233 iterations, 1752.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 601737.25,
            "unit": "ns/iter",
            "extra": "232 iterations, 1742.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 665111.043,
            "unit": "ns/iter",
            "extra": "209 iterations, 1576.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 624842.045,
            "unit": "ns/iter",
            "extra": "220 iterations, 1678.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 583329.338,
            "unit": "ns/iter",
            "extra": "240 iterations, 1797.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 566426.96,
            "unit": "ns/iter",
            "extra": "247 iterations, 1851.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 597840.991,
            "unit": "ns/iter",
            "extra": "234 iterations, 1753.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 699948.368,
            "unit": "ns/iter",
            "extra": "209 iterations, 1489.3MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1994321.59,
            "unit": "ns/iter",
            "extra": "39 iterations, 525.8MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 2266448.723,
            "unit": "ns/iter",
            "extra": "65 iterations, 462.7MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 3348731.061,
            "unit": "ns/iter",
            "extra": "33 iterations, 313.1MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 4637959.7,
            "unit": "ns/iter",
            "extra": "30 iterations, 224.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 156558.293,
            "unit": "ns/iter",
            "extra": "1258 iterations, 99.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 197601.362,
            "unit": "ns/iter",
            "extra": "453 iterations, 159.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 254447.076,
            "unit": "ns/iter",
            "extra": "607 iterations, 42.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 523361.457,
            "unit": "ns/iter",
            "extra": "280 iterations, 40.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 648913.265,
            "unit": "ns/iter",
            "extra": "294 iterations, 44.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 109736.501,
            "unit": "ns/iter",
            "extra": "1460 iterations, 49.3MB/s"
          }
        ]
      },
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
          "id": "a9b16d4c2f19f459e3f24be86858aee148a60fc0",
          "message": "Print a syntax graph the way the rest is printed",
          "timestamp": "2026-09-27T19:54:23+02:00",
          "tree_id": "35b6511425509db44f5df5c5b11458162cc830b4",
          "url": "https://github.com/sgatev/lucid/commit/a9b16d4c2f19f459e3f24be86858aee148a60fc0"
        },
        "date": 1790542193961,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 9788.21,
            "unit": "ns/iter",
            "extra": "20000 iterations, 183.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 37493.809,
            "unit": "ns/iter",
            "extra": "3782 iterations, 203.6MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 74428.988,
            "unit": "ns/iter",
            "extra": "1816 iterations, 209.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 27270.431,
            "unit": "ns/iter",
            "extra": "5127 iterations, 124.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 31751.849,
            "unit": "ns/iter",
            "extra": "4463 iterations, 240.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 61717.86,
            "unit": "ns/iter",
            "extra": "2222 iterations, 252.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 27113.962,
            "unit": "ns/iter",
            "extra": "4832 iterations, 124.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 111990.14,
            "unit": "ns/iter",
            "extra": "1234 iterations, 68.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 390665.333,
            "unit": "ns/iter",
            "extra": "375 iterations, 8.7MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 344.393,
            "unit": "ns/iter",
            "extra": "540080 iterations, 153.9MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 14793.34,
            "unit": "ns/iter",
            "extra": "9461 iterations, 516.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 27404.044,
            "unit": "ns/iter",
            "extra": "5176 iterations, 568.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 76223.393,
            "unit": "ns/iter",
            "extra": "1851 iterations, 71.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 153504.531,
            "unit": "ns/iter",
            "extra": "947 iterations, 69.9MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 7776.95,
            "unit": "ns/iter",
            "extra": "20000 iterations, 434.9MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 32.11,
            "unit": "ns/iter",
            "extra": "9720506 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 18.407,
            "unit": "ns/iter",
            "extra": "7313187 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 33.335,
            "unit": "ns/iter",
            "extra": "10426525 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 18.063,
            "unit": "ns/iter",
            "extra": "6907809 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 32.431,
            "unit": "ns/iter",
            "extra": "11247351 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 19.754,
            "unit": "ns/iter",
            "extra": "6998804 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 12.167,
            "unit": "ns/iter",
            "extra": "9143877 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 5.692,
            "unit": "ns/iter",
            "extra": "19434322 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 7.032,
            "unit": "ns/iter",
            "extra": "18225509 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 32.079,
            "unit": "ns/iter",
            "extra": "10645578 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 12.811,
            "unit": "ns/iter",
            "extra": "8118785 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 9.092,
            "unit": "ns/iter",
            "extra": "24398039 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 4.239,
            "unit": "ns/iter",
            "extra": "37260055 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 4.394,
            "unit": "ns/iter",
            "extra": "24438674 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.495,
            "unit": "ns/iter",
            "extra": "200000000 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.178,
            "unit": "ns/iter",
            "extra": "63571347 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 5.432,
            "unit": "ns/iter",
            "extra": "26262105 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 657444.769,
            "unit": "ns/iter",
            "extra": "212 iterations, 1594.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 601203.529,
            "unit": "ns/iter",
            "extra": "208 iterations, 1744.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 667861.11,
            "unit": "ns/iter",
            "extra": "210 iterations, 1569.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 683629.78,
            "unit": "ns/iter",
            "extra": "218 iterations, 1533.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 582195.077,
            "unit": "ns/iter",
            "extra": "220 iterations, 1801.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 566388.094,
            "unit": "ns/iter",
            "extra": "245 iterations, 1851.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 634504.63,
            "unit": "ns/iter",
            "extra": "216 iterations, 1652.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 777073.96,
            "unit": "ns/iter",
            "extra": "200 iterations, 1341.5MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 2008453.036,
            "unit": "ns/iter",
            "extra": "55 iterations, 522.1MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1944036.538,
            "unit": "ns/iter",
            "extra": "65 iterations, 539.4MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2832874.023,
            "unit": "ns/iter",
            "extra": "43 iterations, 370.1MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 3578718.459,
            "unit": "ns/iter",
            "extra": "37 iterations, 291.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 81287.194,
            "unit": "ns/iter",
            "extra": "1686 iterations, 191.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 158686.536,
            "unit": "ns/iter",
            "extra": "973 iterations, 198.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 222543.259,
            "unit": "ns/iter",
            "extra": "707 iterations, 48.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 413336.42,
            "unit": "ns/iter",
            "extra": "324 iterations, 51.7MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 462829.453,
            "unit": "ns/iter",
            "extra": "333 iterations, 62.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 76884.768,
            "unit": "ns/iter",
            "extra": "2073 iterations, 70.3MB/s"
          }
        ]
      },
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
          "id": "60415664d2278ef4918d9db7a003e118f8f7b3e5",
          "message": "Show throughput by stage on the benchmark page",
          "timestamp": "2026-09-28T06:42:04+02:00",
          "tree_id": "6408d728e086b0d03e5b614bae94ae84e7b9fca4",
          "url": "https://github.com/sgatev/lucid/commit/60415664d2278ef4918d9db7a003e118f8f7b3e5"
        },
        "date": 1790570982668,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 10399.219,
            "unit": "ns/iter",
            "extra": "20000 iterations, 172.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 37615.222,
            "unit": "ns/iter",
            "extra": "3601 iterations, 203.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 79060.385,
            "unit": "ns/iter",
            "extra": "1812 iterations, 197.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 27617.57,
            "unit": "ns/iter",
            "extra": "4890 iterations, 122.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 32169.551,
            "unit": "ns/iter",
            "extra": "4276 iterations, 237.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 62825.228,
            "unit": "ns/iter",
            "extra": "2231 iterations, 247.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 27061.934,
            "unit": "ns/iter",
            "extra": "4930 iterations, 125.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 117385.59,
            "unit": "ns/iter",
            "extra": "1204 iterations, 65.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 390557.648,
            "unit": "ns/iter",
            "extra": "352 iterations, 8.7MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 328.177,
            "unit": "ns/iter",
            "extra": "592294 iterations, 161.5MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 14249.809,
            "unit": "ns/iter",
            "extra": "9834 iterations, 535.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 27218.902,
            "unit": "ns/iter",
            "extra": "5267 iterations, 572.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 73547.314,
            "unit": "ns/iter",
            "extra": "1852 iterations, 73.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 148328.331,
            "unit": "ns/iter",
            "extra": "933 iterations, 72.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 7955.623,
            "unit": "ns/iter",
            "extra": "20000 iterations, 425.1MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 34.105,
            "unit": "ns/iter",
            "extra": "9196107 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 19.696,
            "unit": "ns/iter",
            "extra": "6023340 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 35.018,
            "unit": "ns/iter",
            "extra": "10531264 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 20.348,
            "unit": "ns/iter",
            "extra": "6342925 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 35.149,
            "unit": "ns/iter",
            "extra": "10904169 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 21.519,
            "unit": "ns/iter",
            "extra": "4548592 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 14.977,
            "unit": "ns/iter",
            "extra": "8205108 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 5.634,
            "unit": "ns/iter",
            "extra": "24182752 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 5.256,
            "unit": "ns/iter",
            "extra": "19066210 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 29.177,
            "unit": "ns/iter",
            "extra": "9671458 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 5.76,
            "unit": "ns/iter",
            "extra": "13211286 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 4.076,
            "unit": "ns/iter",
            "extra": "32792322 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 3.546,
            "unit": "ns/iter",
            "extra": "42900371 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 5.86,
            "unit": "ns/iter",
            "extra": "32621675 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.498,
            "unit": "ns/iter",
            "extra": "248197611 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.181,
            "unit": "ns/iter",
            "extra": "63624303 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 5.64,
            "unit": "ns/iter",
            "extra": "29458954 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 600475.573,
            "unit": "ns/iter",
            "extra": "232 iterations, 1746.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 602430.435,
            "unit": "ns/iter",
            "extra": "230 iterations, 1740.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 665324.918,
            "unit": "ns/iter",
            "extra": "208 iterations, 1575.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 627450.077,
            "unit": "ns/iter",
            "extra": "222 iterations, 1671.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 584725.35,
            "unit": "ns/iter",
            "extra": "240 iterations, 1793.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 571098.378,
            "unit": "ns/iter",
            "extra": "241 iterations, 1836.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 597552.126,
            "unit": "ns/iter",
            "extra": "231 iterations, 1754.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 668580.162,
            "unit": "ns/iter",
            "extra": "210 iterations, 1559.1MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1762306.222,
            "unit": "ns/iter",
            "extra": "63 iterations, 595.0MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1822540.541,
            "unit": "ns/iter",
            "extra": "74 iterations, 575.3MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2835389.5,
            "unit": "ns/iter",
            "extra": "46 iterations, 369.8MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 3720737.735,
            "unit": "ns/iter",
            "extra": "34 iterations, 280.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 83757.252,
            "unit": "ns/iter",
            "extra": "1264 iterations, 185.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 162795.811,
            "unit": "ns/iter",
            "extra": "925 iterations, 193.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 226897.563,
            "unit": "ns/iter",
            "extra": "711 iterations, 47.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 412720.948,
            "unit": "ns/iter",
            "extra": "327 iterations, 51.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 477733.28,
            "unit": "ns/iter",
            "extra": "314 iterations, 60.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 78335.448,
            "unit": "ns/iter",
            "extra": "2108 iterations, 69.0MB/s"
          }
        ]
      },
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
          "id": "0262063b31246ad4becd52e513b978e141d78995",
          "message": "Add an example that counts primes while compiling",
          "timestamp": "2026-09-30T22:25:54+02:00",
          "tree_id": "59ede0ee3c93ff2396389986f5de1930e581556d",
          "url": "https://github.com/sgatev/lucid/commit/0262063b31246ad4becd52e513b978e141d78995"
        },
        "date": 1790801286251,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 9913.25,
            "unit": "ns/iter",
            "extra": "10000 iterations, 180.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 38041.956,
            "unit": "ns/iter",
            "extra": "3461 iterations, 200.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 74853.706,
            "unit": "ns/iter",
            "extra": "1853 iterations, 208.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 27266.384,
            "unit": "ns/iter",
            "extra": "5137 iterations, 124.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 43338.749,
            "unit": "ns/iter",
            "extra": "3285 iterations, 176.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 84713.471,
            "unit": "ns/iter",
            "extra": "1614 iterations, 183.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 32584.745,
            "unit": "ns/iter",
            "extra": "4190 iterations, 103.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 129096.021,
            "unit": "ns/iter",
            "extra": "1087 iterations, 59.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 396921.92,
            "unit": "ns/iter",
            "extra": "349 iterations, 8.5MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 325.777,
            "unit": "ns/iter",
            "extra": "599442 iterations, 162.7MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 14418.269,
            "unit": "ns/iter",
            "extra": "9229 iterations, 529.5MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 27574.558,
            "unit": "ns/iter",
            "extra": "4990 iterations, 564.7MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 74290.63,
            "unit": "ns/iter",
            "extra": "1769 iterations, 83.5MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 151078.058,
            "unit": "ns/iter",
            "extra": "932 iterations, 81.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 7797.931,
            "unit": "ns/iter",
            "extra": "20000 iterations, 433.7MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 35.726,
            "unit": "ns/iter",
            "extra": "10335567 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 17.958,
            "unit": "ns/iter",
            "extra": "6937862 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 29.65,
            "unit": "ns/iter",
            "extra": "11954232 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 18.183,
            "unit": "ns/iter",
            "extra": "9587399 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 33.29,
            "unit": "ns/iter",
            "extra": "11290436 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 19.653,
            "unit": "ns/iter",
            "extra": "7542392 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 14.687,
            "unit": "ns/iter",
            "extra": "10145448 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 5.707,
            "unit": "ns/iter",
            "extra": "26066108 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 7.132,
            "unit": "ns/iter",
            "extra": "43907222 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 33.118,
            "unit": "ns/iter",
            "extra": "11030715 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 14.141,
            "unit": "ns/iter",
            "extra": "9426498 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 11.2,
            "unit": "ns/iter",
            "extra": "18192253 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 4.941,
            "unit": "ns/iter",
            "extra": "26571979 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 6.441,
            "unit": "ns/iter",
            "extra": "21154293 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.537,
            "unit": "ns/iter",
            "extra": "200000000 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.199,
            "unit": "ns/iter",
            "extra": "63701512 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 5.841,
            "unit": "ns/iter",
            "extra": "26081487 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 607020.833,
            "unit": "ns/iter",
            "extra": "228 iterations, 1727.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 654243.059,
            "unit": "ns/iter",
            "extra": "204 iterations, 1602.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 750384.165,
            "unit": "ns/iter",
            "extra": "200 iterations, 1397.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 628095.625,
            "unit": "ns/iter",
            "extra": "200 iterations, 1669.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 635679.188,
            "unit": "ns/iter",
            "extra": "213 iterations, 1649.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 614306.362,
            "unit": "ns/iter",
            "extra": "224 iterations, 1706.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 653835.274,
            "unit": "ns/iter",
            "extra": "215 iterations, 1603.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 749264.585,
            "unit": "ns/iter",
            "extra": "200 iterations, 1392.5MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1957581.686,
            "unit": "ns/iter",
            "extra": "51 iterations, 535.6MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 2133913.462,
            "unit": "ns/iter",
            "extra": "65 iterations, 491.4MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 3094240.733,
            "unit": "ns/iter",
            "extra": "45 iterations, 338.9MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 4025253.788,
            "unit": "ns/iter",
            "extra": "33 iterations, 259.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 116338.262,
            "unit": "ns/iter",
            "extra": "1547 iterations, 133.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 233829.485,
            "unit": "ns/iter",
            "extra": "747 iterations, 134.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 358130.479,
            "unit": "ns/iter",
            "extra": "403 iterations, 34.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 707110.625,
            "unit": "ns/iter",
            "extra": "200 iterations, 34.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 697939.617,
            "unit": "ns/iter",
            "extra": "256 iterations, 48.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 106321.082,
            "unit": "ns/iter",
            "extra": "1612 iterations, 59.3MB/s"
          }
        ]
      },
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
          "id": "e38ef08be3095770be8b29a7683b64b529ec27ee",
          "message": "Turn division by zero into an error",
          "timestamp": "2026-10-01T07:16:34+02:00",
          "tree_id": "edacc7e9756544dc42d1696090b4d4836678a070",
          "url": "https://github.com/sgatev/lucid/commit/e38ef08be3095770be8b29a7683b64b529ec27ee"
        },
        "date": 1790833258801,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 9040.167,
            "unit": "ns/iter",
            "extra": "10000 iterations, 198.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 34499.234,
            "unit": "ns/iter",
            "extra": "4077 iterations, 221.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 76072.665,
            "unit": "ns/iter",
            "extra": "2066 iterations, 204.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 26290.227,
            "unit": "ns/iter",
            "extra": "5038 iterations, 128.6MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 51291.339,
            "unit": "ns/iter",
            "extra": "2921 iterations, 148.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 89877.869,
            "unit": "ns/iter",
            "extra": "1539 iterations, 173.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 46461.086,
            "unit": "ns/iter",
            "extra": "2861 iterations, 72.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 200224.638,
            "unit": "ns/iter",
            "extra": "690 iterations, 38.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 575909.375,
            "unit": "ns/iter",
            "extra": "200 iterations, 5.9MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 358.566,
            "unit": "ns/iter",
            "extra": "506302 iterations, 147.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 15257.033,
            "unit": "ns/iter",
            "extra": "9041 iterations, 500.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 28358.23,
            "unit": "ns/iter",
            "extra": "4604 iterations, 549.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 74896.088,
            "unit": "ns/iter",
            "extra": "1804 iterations, 82.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 156640.234,
            "unit": "ns/iter",
            "extra": "815 iterations, 78.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 9166.552,
            "unit": "ns/iter",
            "extra": "20000 iterations, 369.0MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 30.755,
            "unit": "ns/iter",
            "extra": "3825946 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 26.868,
            "unit": "ns/iter",
            "extra": "6531538 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 33.836,
            "unit": "ns/iter",
            "extra": "5442899 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 23.336,
            "unit": "ns/iter",
            "extra": "4803479 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 39.747,
            "unit": "ns/iter",
            "extra": "6740396 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 30.493,
            "unit": "ns/iter",
            "extra": "4083666 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 23.557,
            "unit": "ns/iter",
            "extra": "3944662 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 6.391,
            "unit": "ns/iter",
            "extra": "12929541 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 8.098,
            "unit": "ns/iter",
            "extra": "13958773 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 29.825,
            "unit": "ns/iter",
            "extra": "8605829 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 12.249,
            "unit": "ns/iter",
            "extra": "10427885 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 8.97,
            "unit": "ns/iter",
            "extra": "14186193 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 5.082,
            "unit": "ns/iter",
            "extra": "31638117 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 6.116,
            "unit": "ns/iter",
            "extra": "24192330 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.593,
            "unit": "ns/iter",
            "extra": "200000000 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.487,
            "unit": "ns/iter",
            "extra": "45574147 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 6.213,
            "unit": "ns/iter",
            "extra": "23588712 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 670583.54,
            "unit": "ns/iter",
            "extra": "200 iterations, 1563.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 650608.214,
            "unit": "ns/iter",
            "extra": "206 iterations, 1611.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 723703.476,
            "unit": "ns/iter",
            "extra": "206 iterations, 1448.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 707270.126,
            "unit": "ns/iter",
            "extra": "207 iterations, 1482.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 654713.263,
            "unit": "ns/iter",
            "extra": "262 iterations, 1601.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 617276.256,
            "unit": "ns/iter",
            "extra": "219 iterations, 1698.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 661443.909,
            "unit": "ns/iter",
            "extra": "208 iterations, 1585.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 752802.448,
            "unit": "ns/iter",
            "extra": "201 iterations, 1385.9MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1911257.508,
            "unit": "ns/iter",
            "extra": "61 iterations, 548.6MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 2007561.882,
            "unit": "ns/iter",
            "extra": "68 iterations, 522.3MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2996948.628,
            "unit": "ns/iter",
            "extra": "43 iterations, 349.9MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 3955944.455,
            "unit": "ns/iter",
            "extra": "33 iterations, 263.7MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 124107.122,
            "unit": "ns/iter",
            "extra": "1100 iterations, 125.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 221694.556,
            "unit": "ns/iter",
            "extra": "880 iterations, 142.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 354046.384,
            "unit": "ns/iter",
            "extra": "424 iterations, 34.7MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 649415.64,
            "unit": "ns/iter",
            "extra": "203 iterations, 37.7MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 565432.439,
            "unit": "ns/iter",
            "extra": "280 iterations, 59.7MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 92728.412,
            "unit": "ns/iter",
            "extra": "1712 iterations, 68.0MB/s"
          }
        ]
      },
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
          "id": "a4f2a95b2c37d8ae653c1fc6e30c7917ab68fb0f",
          "message": "Mark the benchmarks testonly",
          "timestamp": "2026-10-02T19:29:25+02:00",
          "tree_id": "fbb520adfc3fedf6e766ce185ab218a2e17cb816",
          "url": "https://github.com/sgatev/lucid/commit/a4f2a95b2c37d8ae653c1fc6e30c7917ab68fb0f"
        },
        "date": 1790962638626,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 13631.558,
            "unit": "ns/iter",
            "extra": "10000 iterations, 131.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 52925.476,
            "unit": "ns/iter",
            "extra": "3680 iterations, 144.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 100032.588,
            "unit": "ns/iter",
            "extra": "1533 iterations, 155.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 34028.628,
            "unit": "ns/iter",
            "extra": "4100 iterations, 99.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 54746.636,
            "unit": "ns/iter",
            "extra": "3047 iterations, 139.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 115800.053,
            "unit": "ns/iter",
            "extra": "1416 iterations, 134.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 49009.793,
            "unit": "ns/iter",
            "extra": "3374 iterations, 69.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 159111.002,
            "unit": "ns/iter",
            "extra": "893 iterations, 48.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 560698.489,
            "unit": "ns/iter",
            "extra": "309 iterations, 6.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgDiamonds30",
            "value": 450475.219,
            "unit": "ns/iter",
            "extra": "343 iterations, 21.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds15",
            "value": 5572237,
            "unit": "ns/iter",
            "extra": "16 iterations, 1.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds30",
            "value": 19368741.6,
            "unit": "ns/iter",
            "extra": "5 iterations, 0.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateBranching40",
            "value": 509655.415,
            "unit": "ns/iter",
            "extra": "200 iterations, 6.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLoopCarried30",
            "value": 285224.433,
            "unit": "ns/iter",
            "extra": "427 iterations, 8.3MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 471.41,
            "unit": "ns/iter",
            "extra": "223545 iterations, 112.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 18318.713,
            "unit": "ns/iter",
            "extra": "7783 iterations, 416.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 33337.609,
            "unit": "ns/iter",
            "extra": "4989 iterations, 467.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 99389.679,
            "unit": "ns/iter",
            "extra": "1882 iterations, 62.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 242402.989,
            "unit": "ns/iter",
            "extra": "463 iterations, 50.7MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 11014.821,
            "unit": "ns/iter",
            "extra": "10000 iterations, 307.0MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 26.847,
            "unit": "ns/iter",
            "extra": "2875075 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 27.635,
            "unit": "ns/iter",
            "extra": "4326596 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 51.252,
            "unit": "ns/iter",
            "extra": "9790751 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 28.696,
            "unit": "ns/iter",
            "extra": "6640282 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 43.381,
            "unit": "ns/iter",
            "extra": "8250744 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 29.56,
            "unit": "ns/iter",
            "extra": "4904357 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 22.336,
            "unit": "ns/iter",
            "extra": "7295434 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 7.154,
            "unit": "ns/iter",
            "extra": "23717425 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 9.739,
            "unit": "ns/iter",
            "extra": "17534062 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 47.132,
            "unit": "ns/iter",
            "extra": "7200812 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 25.033,
            "unit": "ns/iter",
            "extra": "5430802 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 18.074,
            "unit": "ns/iter",
            "extra": "11666180 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 6.99,
            "unit": "ns/iter",
            "extra": "23112005 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 11.417,
            "unit": "ns/iter",
            "extra": "20805920 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.717,
            "unit": "ns/iter",
            "extra": "100000000 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.44,
            "unit": "ns/iter",
            "extra": "45853395 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 5.962,
            "unit": "ns/iter",
            "extra": "24178054 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 672025.505,
            "unit": "ns/iter",
            "extra": "214 iterations, 1560.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 685344.79,
            "unit": "ns/iter",
            "extra": "200 iterations, 1529.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 741352.085,
            "unit": "ns/iter",
            "extra": "200 iterations, 1414.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 678696.579,
            "unit": "ns/iter",
            "extra": "202 iterations, 1544.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 637155.543,
            "unit": "ns/iter",
            "extra": "221 iterations, 1645.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 616267.621,
            "unit": "ns/iter",
            "extra": "227 iterations, 1701.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 719474.725,
            "unit": "ns/iter",
            "extra": "211 iterations, 1457.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 809714.165,
            "unit": "ns/iter",
            "extra": "200 iterations, 1288.5MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 2087718.75,
            "unit": "ns/iter",
            "extra": "36 iterations, 502.3MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 2089858.333,
            "unit": "ns/iter",
            "extra": "60 iterations, 501.7MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 3054575.911,
            "unit": "ns/iter",
            "extra": "45 iterations, 343.3MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 3931421.727,
            "unit": "ns/iter",
            "extra": "33 iterations, 265.4MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 125052.541,
            "unit": "ns/iter",
            "extra": "1548 iterations, 124.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 227291.724,
            "unit": "ns/iter",
            "extra": "735 iterations, 138.7MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 349000,
            "unit": "ns/iter",
            "extra": "407 iterations, 35.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 741023.606,
            "unit": "ns/iter",
            "extra": "203 iterations, 33.0MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 645019.991,
            "unit": "ns/iter",
            "extra": "223 iterations, 52.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 109638.163,
            "unit": "ns/iter",
            "extra": "1532 iterations, 57.5MB/s"
          }
        ]
      },
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
          "id": "5875ed241bf2265be8f7171fda93ed4bfcc0c32f",
          "message": "Refer to labels by number rather than by name",
          "timestamp": "2026-10-02T23:50:19+02:00",
          "tree_id": "09a424693bfbcf69e9988de59e65daa413e6087d",
          "url": "https://github.com/sgatev/lucid/commit/5875ed241bf2265be8f7171fda93ed4bfcc0c32f"
        },
        "date": 1790978440682,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 10616.083,
            "unit": "ns/iter",
            "extra": "10000 iterations, 168.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 40206.223,
            "unit": "ns/iter",
            "extra": "3060 iterations, 189.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 75975.031,
            "unit": "ns/iter",
            "extra": "1879 iterations, 204.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 28367.712,
            "unit": "ns/iter",
            "extra": "4562 iterations, 119.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 42664.905,
            "unit": "ns/iter",
            "extra": "4306 iterations, 179.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 71633.93,
            "unit": "ns/iter",
            "extra": "1857 iterations, 217.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 40520.857,
            "unit": "ns/iter",
            "extra": "4423 iterations, 83.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 165705.106,
            "unit": "ns/iter",
            "extra": "1007 iterations, 46.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 486596.092,
            "unit": "ns/iter",
            "extra": "271 iterations, 7.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgDiamonds30",
            "value": 329123.368,
            "unit": "ns/iter",
            "extra": "383 iterations, 29.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds15",
            "value": 6284904.714,
            "unit": "ns/iter",
            "extra": "14 iterations, 1.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds30",
            "value": 21852437.5,
            "unit": "ns/iter",
            "extra": "4 iterations, 0.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateBranching40",
            "value": 523161.758,
            "unit": "ns/iter",
            "extra": "297 iterations, 6.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLoopCarried30",
            "value": 233845.874,
            "unit": "ns/iter",
            "extra": "412 iterations, 10.1MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf10Functions",
            "value": 940.5,
            "unit": "ns/iter",
            "extra": "115427 iterations, 139.3MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf1000Functions",
            "value": 1072.038,
            "unit": "ns/iter",
            "extra": "140639 iterations, 124.1MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 383.97,
            "unit": "ns/iter",
            "extra": "342664 iterations, 138.0MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 7154.569,
            "unit": "ns/iter",
            "extra": "20000 iterations, 1067.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 16030.149,
            "unit": "ns/iter",
            "extra": "9840 iterations, 971.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 32231.829,
            "unit": "ns/iter",
            "extra": "4061 iterations, 192.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 63535.852,
            "unit": "ns/iter",
            "extra": "1906 iterations, 193.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenBranches64",
            "value": 133405.873,
            "unit": "ns/iter",
            "extra": "1162 iterations, 92.0MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenChainedValues512",
            "value": 42149.098,
            "unit": "ns/iter",
            "extra": "3069 iterations, 369.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 4248.253,
            "unit": "ns/iter",
            "extra": "27761 iterations, 796.1MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 32.962,
            "unit": "ns/iter",
            "extra": "8312428 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 22.516,
            "unit": "ns/iter",
            "extra": "6930893 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 56.575,
            "unit": "ns/iter",
            "extra": "10704318 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 24.377,
            "unit": "ns/iter",
            "extra": "5474152 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 38.504,
            "unit": "ns/iter",
            "extra": "9001671 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 25.275,
            "unit": "ns/iter",
            "extra": "2614403 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 26.06,
            "unit": "ns/iter",
            "extra": "6758237 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 7.063,
            "unit": "ns/iter",
            "extra": "22131763 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 10.686,
            "unit": "ns/iter",
            "extra": "10293014 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 36.214,
            "unit": "ns/iter",
            "extra": "9448499 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 12.093,
            "unit": "ns/iter",
            "extra": "10996420 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 10.851,
            "unit": "ns/iter",
            "extra": "14353085 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 5.748,
            "unit": "ns/iter",
            "extra": "24828748 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 7.266,
            "unit": "ns/iter",
            "extra": "18408234 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.59,
            "unit": "ns/iter",
            "extra": "205054472 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 2.4,
            "unit": "ns/iter",
            "extra": "57323202 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 5.846,
            "unit": "ns/iter",
            "extra": "22758983 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 642607.753,
            "unit": "ns/iter",
            "extra": "215 iterations, 1631.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 642162.288,
            "unit": "ns/iter",
            "extra": "219 iterations, 1632.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 713776.46,
            "unit": "ns/iter",
            "extra": "200 iterations, 1468.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 665314.01,
            "unit": "ns/iter",
            "extra": "207 iterations, 1576.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 635914.968,
            "unit": "ns/iter",
            "extra": "221 iterations, 1648.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 648182.944,
            "unit": "ns/iter",
            "extra": "215 iterations, 1617.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 646252.054,
            "unit": "ns/iter",
            "extra": "203 iterations, 1622.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 708713.54,
            "unit": "ns/iter",
            "extra": "200 iterations, 1472.1MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1910082.076,
            "unit": "ns/iter",
            "extra": "66 iterations, 549.0MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 2011933.448,
            "unit": "ns/iter",
            "extra": "67 iterations, 521.2MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 3038985.795,
            "unit": "ns/iter",
            "extra": "44 iterations, 345.0MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 3900099.265,
            "unit": "ns/iter",
            "extra": "34 iterations, 267.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 92793.482,
            "unit": "ns/iter",
            "extra": "1675 iterations, 167.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 205628.576,
            "unit": "ns/iter",
            "extra": "874 iterations, 153.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 243498.023,
            "unit": "ns/iter",
            "extra": "569 iterations, 50.4MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 487391.669,
            "unit": "ns/iter",
            "extra": "245 iterations, 50.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 573639.045,
            "unit": "ns/iter",
            "extra": "267 iterations, 58.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 89449.638,
            "unit": "ns/iter",
            "extra": "1864 iterations, 70.4MB/s"
          }
        ]
      },
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
          "id": "143073e38055003f810d35020f9283887be79e71",
          "message": "Update CI to use Namespace runners",
          "timestamp": "2026-10-03T17:41:51+02:00",
          "tree_id": "a18c5ba093249f24d2705b424ac64ba5cadc66bf",
          "url": "https://github.com/sgatev/lucid/commit/143073e38055003f810d35020f9283887be79e71"
        },
        "date": 1791042395182,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 4438.982,
            "unit": "ns/iter",
            "extra": "30329 iterations, 403.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 17380.739,
            "unit": "ns/iter",
            "extra": "8117 iterations, 439.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 34269.007,
            "unit": "ns/iter",
            "extra": "3685 iterations, 454.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 14882.038,
            "unit": "ns/iter",
            "extra": "9147 iterations, 227.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 22409.297,
            "unit": "ns/iter",
            "extra": "6304 iterations, 340.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 42174.485,
            "unit": "ns/iter",
            "extra": "3336 iterations, 369.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 17873.986,
            "unit": "ns/iter",
            "extra": "7727 iterations, 189.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 59087.739,
            "unit": "ns/iter",
            "extra": "2317 iterations, 129.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 166013.889,
            "unit": "ns/iter",
            "extra": "741 iterations, 20.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgDiamonds30",
            "value": 164796.192,
            "unit": "ns/iter",
            "extra": "847 iterations, 58.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds15",
            "value": 402006.389,
            "unit": "ns/iter",
            "extra": "339 iterations, 21.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds30",
            "value": 847529.375,
            "unit": "ns/iter",
            "extra": "200 iterations, 19.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateBranching40",
            "value": 104439.734,
            "unit": "ns/iter",
            "extra": "1315 iterations, 31.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLoopCarried30",
            "value": 76081.208,
            "unit": "ns/iter",
            "extra": "1843 iterations, 31.0MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf10Functions",
            "value": 647.658,
            "unit": "ns/iter",
            "extra": "175266 iterations, 202.3MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf1000Functions",
            "value": 652.516,
            "unit": "ns/iter",
            "extra": "212375 iterations, 203.8MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 257.048,
            "unit": "ns/iter",
            "extra": "538625 iterations, 206.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 4859.652,
            "unit": "ns/iter",
            "extra": "27928 iterations, 1571.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 9170.702,
            "unit": "ns/iter",
            "extra": "20000 iterations, 1697.9MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 16446.995,
            "unit": "ns/iter",
            "extra": "8404 iterations, 377.0MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 32206.349,
            "unit": "ns/iter",
            "extra": "4367 iterations, 381.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenBranches64",
            "value": 39371.647,
            "unit": "ns/iter",
            "extra": "3579 iterations, 311.9MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenChainedValues512",
            "value": 13672.4,
            "unit": "ns/iter",
            "extra": "9520 iterations, 1138.9MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 2886.478,
            "unit": "ns/iter",
            "extra": "48421 iterations, 1171.7MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 19.501,
            "unit": "ns/iter",
            "extra": "13685127 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 5.994,
            "unit": "ns/iter",
            "extra": "21903662 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 21.94,
            "unit": "ns/iter",
            "extra": "13470226 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 5.745,
            "unit": "ns/iter",
            "extra": "19756805 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 22.604,
            "unit": "ns/iter",
            "extra": "19122744 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 5.227,
            "unit": "ns/iter",
            "extra": "19472842 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 3.735,
            "unit": "ns/iter",
            "extra": "36701653 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 2.687,
            "unit": "ns/iter",
            "extra": "54439396 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 2.885,
            "unit": "ns/iter",
            "extra": "33278859 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 20.422,
            "unit": "ns/iter",
            "extra": "17289019 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 3.465,
            "unit": "ns/iter",
            "extra": "33181252 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 2.515,
            "unit": "ns/iter",
            "extra": "53302022 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 2.363,
            "unit": "ns/iter",
            "extra": "59640046 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 2.774,
            "unit": "ns/iter",
            "extra": "48529672 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.361,
            "unit": "ns/iter",
            "extra": "376493792 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 1.621,
            "unit": "ns/iter",
            "extra": "83744579 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 4.12,
            "unit": "ns/iter",
            "extra": "33508190 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 299854.258,
            "unit": "ns/iter",
            "extra": "462 iterations, 3496.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 325347.221,
            "unit": "ns/iter",
            "extra": "417 iterations, 3222.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 325397.849,
            "unit": "ns/iter",
            "extra": "403 iterations, 3222.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 366148.507,
            "unit": "ns/iter",
            "extra": "335 iterations, 2863.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 311499.467,
            "unit": "ns/iter",
            "extra": "390 iterations, 3366.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 324459.808,
            "unit": "ns/iter",
            "extra": "396 iterations, 3231.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 343206.082,
            "unit": "ns/iter",
            "extra": "352 iterations, 3054.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 452319.039,
            "unit": "ns/iter",
            "extra": "309 iterations, 2306.6MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1318445.733,
            "unit": "ns/iter",
            "extra": "86 iterations, 795.3MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1374017.734,
            "unit": "ns/iter",
            "extra": "94 iterations, 763.1MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2183197.368,
            "unit": "ns/iter",
            "extra": "57 iterations, 480.3MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 2836909.091,
            "unit": "ns/iter",
            "extra": "44 iterations, 367.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 68123.286,
            "unit": "ns/iter",
            "extra": "2358 iterations, 228.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 130570.107,
            "unit": "ns/iter",
            "extra": "1405 iterations, 241.4MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 147327.432,
            "unit": "ns/iter",
            "extra": "1059 iterations, 83.4MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 299115.143,
            "unit": "ns/iter",
            "extra": "503 iterations, 81.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 317711.667,
            "unit": "ns/iter",
            "extra": "375 iterations, 106.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 62436.472,
            "unit": "ns/iter",
            "extra": "2937 iterations, 100.9MB/s"
          }
        ]
      },
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
          "id": "87b2b2bedd27c83a263eeb311e89e867a985dbbe",
          "message": "Update README",
          "timestamp": "2026-10-03T18:21:12+02:00",
          "tree_id": "54c10e588eb84250d8dc5cf0360d27b6bf3f2e26",
          "url": "https://github.com/sgatev/lucid/commit/87b2b2bedd27c83a263eeb311e89e867a985dbbe"
        },
        "date": 1791044748304,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 4300.135,
            "unit": "ns/iter",
            "extra": "31301 iterations, 416.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 17077.818,
            "unit": "ns/iter",
            "extra": "8242 iterations, 447.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 33435.002,
            "unit": "ns/iter",
            "extra": "3695 iterations, 465.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 14511.879,
            "unit": "ns/iter",
            "extra": "9102 iterations, 233.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 22108.452,
            "unit": "ns/iter",
            "extra": "5955 iterations, 345.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 42013.535,
            "unit": "ns/iter",
            "extra": "3217 iterations, 370.6MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 17887.88,
            "unit": "ns/iter",
            "extra": "7822 iterations, 189.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 60483.221,
            "unit": "ns/iter",
            "extra": "2297 iterations, 126.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 163967.071,
            "unit": "ns/iter",
            "extra": "844 iterations, 20.6MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgDiamonds30",
            "value": 163585.386,
            "unit": "ns/iter",
            "extra": "853 iterations, 59.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds15",
            "value": 407585.267,
            "unit": "ns/iter",
            "extra": "345 iterations, 20.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds30",
            "value": 836790.205,
            "unit": "ns/iter",
            "extra": "200 iterations, 19.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateBranching40",
            "value": 103833.112,
            "unit": "ns/iter",
            "extra": "1316 iterations, 31.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLoopCarried30",
            "value": 75129.456,
            "unit": "ns/iter",
            "extra": "1842 iterations, 31.4MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf10Functions",
            "value": 620.788,
            "unit": "ns/iter",
            "extra": "177769 iterations, 211.0MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf1000Functions",
            "value": 620.569,
            "unit": "ns/iter",
            "extra": "219150 iterations, 214.3MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 216.073,
            "unit": "ns/iter",
            "extra": "647511 iterations, 245.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 4743.599,
            "unit": "ns/iter",
            "extra": "26137 iterations, 1609.5MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 9163.66,
            "unit": "ns/iter",
            "extra": "20000 iterations, 1699.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 16847.11,
            "unit": "ns/iter",
            "extra": "8281 iterations, 368.0MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 32807.845,
            "unit": "ns/iter",
            "extra": "4131 iterations, 374.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenBranches64",
            "value": 39893.203,
            "unit": "ns/iter",
            "extra": "3580 iterations, 307.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenChainedValues512",
            "value": 13414.212,
            "unit": "ns/iter",
            "extra": "10000 iterations, 1160.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 2740.212,
            "unit": "ns/iter",
            "extra": "51253 iterations, 1234.2MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 21.275,
            "unit": "ns/iter",
            "extra": "16792611 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 4.551,
            "unit": "ns/iter",
            "extra": "30198173 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 25.859,
            "unit": "ns/iter",
            "extra": "19475212 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 5.881,
            "unit": "ns/iter",
            "extra": "21747711 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 21.697,
            "unit": "ns/iter",
            "extra": "16842358 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 5.058,
            "unit": "ns/iter",
            "extra": "36840490 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 3.775,
            "unit": "ns/iter",
            "extra": "38045628 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 2.678,
            "unit": "ns/iter",
            "extra": "53654401 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 2.861,
            "unit": "ns/iter",
            "extra": "66937604 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 20.389,
            "unit": "ns/iter",
            "extra": "18739021 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 3.564,
            "unit": "ns/iter",
            "extra": "39571307 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 2.542,
            "unit": "ns/iter",
            "extra": "51217963 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 2.371,
            "unit": "ns/iter",
            "extra": "60875070 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 2.759,
            "unit": "ns/iter",
            "extra": "49258206 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.36,
            "unit": "ns/iter",
            "extra": "363852548 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 1.603,
            "unit": "ns/iter",
            "extra": "86087624 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 4.033,
            "unit": "ns/iter",
            "extra": "36880519 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 306962.57,
            "unit": "ns/iter",
            "extra": "423 iterations, 3415.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 325530.471,
            "unit": "ns/iter",
            "extra": "361 iterations, 3221.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 320754.798,
            "unit": "ns/iter",
            "extra": "382 iterations, 3268.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 326225.347,
            "unit": "ns/iter",
            "extra": "409 iterations, 3214.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 309000.912,
            "unit": "ns/iter",
            "extra": "457 iterations, 3393.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 313577.947,
            "unit": "ns/iter",
            "extra": "379 iterations, 3343.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 334733.931,
            "unit": "ns/iter",
            "extra": "363 iterations, 3132.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 451627.977,
            "unit": "ns/iter",
            "extra": "308 iterations, 2310.2MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1294590,
            "unit": "ns/iter",
            "extra": "100 iterations, 810.0MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1376446.97,
            "unit": "ns/iter",
            "extra": "99 iterations, 761.8MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2156202.18,
            "unit": "ns/iter",
            "extra": "61 iterations, 486.3MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 2802764.18,
            "unit": "ns/iter",
            "extra": "50 iterations, 372.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 69911.73,
            "unit": "ns/iter",
            "extra": "2701 iterations, 222.7MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 128535.203,
            "unit": "ns/iter",
            "extra": "1444 iterations, 245.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 149856.962,
            "unit": "ns/iter",
            "extra": "1088 iterations, 81.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 313681.739,
            "unit": "ns/iter",
            "extra": "387 iterations, 78.0MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 374580.42,
            "unit": "ns/iter",
            "extra": "486 iterations, 90.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 66817.381,
            "unit": "ns/iter",
            "extra": "2949 iterations, 94.3MB/s"
          }
        ]
      },
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
          "id": "e1f5e4b3a3bf210285414277103898d5f51235d1",
          "message": "Update README",
          "timestamp": "2026-10-03T19:13:57+02:00",
          "tree_id": "90664b7f7b02c9b53334eebd45f1d9bcba399cd6",
          "url": "https://github.com/sgatev/lucid/commit/e1f5e4b3a3bf210285414277103898d5f51235d1"
        },
        "date": 1791047914645,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 4566.578,
            "unit": "ns/iter",
            "extra": "29774 iterations, 392.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 17892.377,
            "unit": "ns/iter",
            "extra": "8011 iterations, 426.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 35241.027,
            "unit": "ns/iter",
            "extra": "4091 iterations, 441.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 15324.458,
            "unit": "ns/iter",
            "extra": "9211 iterations, 220.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 23219.376,
            "unit": "ns/iter",
            "extra": "6203 iterations, 328.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 43116.611,
            "unit": "ns/iter",
            "extra": "3293 iterations, 361.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 18933.856,
            "unit": "ns/iter",
            "extra": "7541 iterations, 178.6MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 64337.105,
            "unit": "ns/iter",
            "extra": "2143 iterations, 118.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 176531.423,
            "unit": "ns/iter",
            "extra": "777 iterations, 19.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgDiamonds30",
            "value": 171923.203,
            "unit": "ns/iter",
            "extra": "784 iterations, 56.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds15",
            "value": 417994.933,
            "unit": "ns/iter",
            "extra": "329 iterations, 20.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds30",
            "value": 875987.295,
            "unit": "ns/iter",
            "extra": "200 iterations, 18.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateBranching40",
            "value": 109354.183,
            "unit": "ns/iter",
            "extra": "1279 iterations, 29.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLoopCarried30",
            "value": 79282.981,
            "unit": "ns/iter",
            "extra": "1799 iterations, 29.7MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf10Functions",
            "value": 637.844,
            "unit": "ns/iter",
            "extra": "169526 iterations, 205.4MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf1000Functions",
            "value": 684.168,
            "unit": "ns/iter",
            "extra": "207779 iterations, 194.4MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 236.115,
            "unit": "ns/iter",
            "extra": "609059 iterations, 224.5MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 4904.862,
            "unit": "ns/iter",
            "extra": "27041 iterations, 1556.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 9297.15,
            "unit": "ns/iter",
            "extra": "20000 iterations, 1674.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 17507.968,
            "unit": "ns/iter",
            "extra": "8006 iterations, 354.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 33067.162,
            "unit": "ns/iter",
            "extra": "4192 iterations, 371.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenBranches64",
            "value": 40085.619,
            "unit": "ns/iter",
            "extra": "3537 iterations, 306.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenChainedValues512",
            "value": 13361.967,
            "unit": "ns/iter",
            "extra": "10000 iterations, 1165.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 3157.874,
            "unit": "ns/iter",
            "extra": "47846 iterations, 1071.0MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 21.862,
            "unit": "ns/iter",
            "extra": "14533438 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 5.947,
            "unit": "ns/iter",
            "extra": "13763665 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 19.578,
            "unit": "ns/iter",
            "extra": "14582699 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 5.828,
            "unit": "ns/iter",
            "extra": "18222442 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 22.617,
            "unit": "ns/iter",
            "extra": "18393522 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 5.531,
            "unit": "ns/iter",
            "extra": "22819733 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 4.072,
            "unit": "ns/iter",
            "extra": "34980323 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 2.939,
            "unit": "ns/iter",
            "extra": "55063003 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 2.967,
            "unit": "ns/iter",
            "extra": "33514535 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 20.879,
            "unit": "ns/iter",
            "extra": "16840164 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 3.725,
            "unit": "ns/iter",
            "extra": "38960131 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 2.716,
            "unit": "ns/iter",
            "extra": "48743683 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 2.491,
            "unit": "ns/iter",
            "extra": "59541732 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 2.871,
            "unit": "ns/iter",
            "extra": "44109534 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.365,
            "unit": "ns/iter",
            "extra": "348764067 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 1.692,
            "unit": "ns/iter",
            "extra": "77637598 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 4.277,
            "unit": "ns/iter",
            "extra": "33518884 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 311747.247,
            "unit": "ns/iter",
            "extra": "454 iterations, 3363.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 342607.597,
            "unit": "ns/iter",
            "extra": "419 iterations, 3060.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 354451.745,
            "unit": "ns/iter",
            "extra": "411 iterations, 2958.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 343425,
            "unit": "ns/iter",
            "extra": "355 iterations, 3053.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 322189.512,
            "unit": "ns/iter",
            "extra": "445 iterations, 3254.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 337165.188,
            "unit": "ns/iter",
            "extra": "394 iterations, 3109.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 361581.443,
            "unit": "ns/iter",
            "extra": "397 iterations, 2899.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 502569.542,
            "unit": "ns/iter",
            "extra": "284 iterations, 2076.0MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1331191.67,
            "unit": "ns/iter",
            "extra": "100 iterations, 787.7MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1413854.592,
            "unit": "ns/iter",
            "extra": "98 iterations, 741.6MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2265002.155,
            "unit": "ns/iter",
            "extra": "58 iterations, 462.9MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 2939345.512,
            "unit": "ns/iter",
            "extra": "41 iterations, 355.0MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 70205.127,
            "unit": "ns/iter",
            "extra": "2495 iterations, 221.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 135641.856,
            "unit": "ns/iter",
            "extra": "1409 iterations, 232.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 153872.236,
            "unit": "ns/iter",
            "extra": "1010 iterations, 79.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 308236.254,
            "unit": "ns/iter",
            "extra": "488 iterations, 79.4MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 380724.93,
            "unit": "ns/iter",
            "extra": "472 iterations, 88.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 64561.611,
            "unit": "ns/iter",
            "extra": "2929 iterations, 97.6MB/s"
          }
        ]
      },
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
          "id": "dba739ca6b04d7a8f685deca9b9562152835b467",
          "message": "Update README",
          "timestamp": "2026-10-03T20:07:28+02:00",
          "tree_id": "11b4c172593bc77972f26689917f9b22938a9106",
          "url": "https://github.com/sgatev/lucid/commit/dba739ca6b04d7a8f685deca9b9562152835b467"
        },
        "date": 1791051136008,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 4300.09,
            "unit": "ns/iter",
            "extra": "30688 iterations, 416.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 16889.437,
            "unit": "ns/iter",
            "extra": "9198 iterations, 452.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 33331.899,
            "unit": "ns/iter",
            "extra": "4184 iterations, 467.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 14357.299,
            "unit": "ns/iter",
            "extra": "9823 iterations, 235.6MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain256",
            "value": 21882.059,
            "unit": "ns/iter",
            "extra": "6528 iterations, 348.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgChain512",
            "value": 42150.714,
            "unit": "ns/iter",
            "extra": "3398 iterations, 369.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgLive64",
            "value": 18098.148,
            "unit": "ns/iter",
            "extra": "7757 iterations, 186.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 64305.158,
            "unit": "ns/iter",
            "extra": "2307 iterations, 118.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 162849.387,
            "unit": "ns/iter",
            "extra": "802 iterations, 20.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/BuildIgDiamonds30",
            "value": 162833.914,
            "unit": "ns/iter",
            "extra": "861 iterations, 59.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds15",
            "value": 391942.272,
            "unit": "ns/iter",
            "extra": "345 iterations, 21.6MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds30",
            "value": 818846.665,
            "unit": "ns/iter",
            "extra": "200 iterations, 19.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateBranching40",
            "value": 101200.08,
            "unit": "ns/iter",
            "extra": "1358 iterations, 32.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLoopCarried30",
            "value": 74577.077,
            "unit": "ns/iter",
            "extra": "1898 iterations, 31.6MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf10Functions",
            "value": 645.04,
            "unit": "ns/iter",
            "extra": "175318 iterations, 203.1MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf1000Functions",
            "value": 648.776,
            "unit": "ns/iter",
            "extra": "214451 iterations, 205.0MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 245.886,
            "unit": "ns/iter",
            "extra": "577667 iterations, 215.5MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 4817.381,
            "unit": "ns/iter",
            "extra": "28842 iterations, 1584.9MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 8948.462,
            "unit": "ns/iter",
            "extra": "20000 iterations, 1740.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 15962.833,
            "unit": "ns/iter",
            "extra": "8528 iterations, 388.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 31130.596,
            "unit": "ns/iter",
            "extra": "4572 iterations, 394.5MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenBranches64",
            "value": 37623.036,
            "unit": "ns/iter",
            "extra": "3734 iterations, 326.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenChainedValues512",
            "value": 12956.696,
            "unit": "ns/iter",
            "extra": "10000 iterations, 1201.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 2747.801,
            "unit": "ns/iter",
            "extra": "50597 iterations, 1230.8MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 18.931,
            "unit": "ns/iter",
            "extra": "15248538 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 5.367,
            "unit": "ns/iter",
            "extra": "22383883 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 24.216,
            "unit": "ns/iter",
            "extra": "18403799 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 8.202,
            "unit": "ns/iter",
            "extra": "12594080 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 20.316,
            "unit": "ns/iter",
            "extra": "15947753 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 5.419,
            "unit": "ns/iter",
            "extra": "24832421 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 3.827,
            "unit": "ns/iter",
            "extra": "35167043 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 2.598,
            "unit": "ns/iter",
            "extra": "60467534 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 2.855,
            "unit": "ns/iter",
            "extra": "59240452 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 20.234,
            "unit": "ns/iter",
            "extra": "18377526 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 3.515,
            "unit": "ns/iter",
            "extra": "38173569 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 2.491,
            "unit": "ns/iter",
            "extra": "49650531 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 2.319,
            "unit": "ns/iter",
            "extra": "59735474 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 2.756,
            "unit": "ns/iter",
            "extra": "47737449 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.361,
            "unit": "ns/iter",
            "extra": "369640623 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 1.591,
            "unit": "ns/iter",
            "extra": "87015108 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 3.961,
            "unit": "ns/iter",
            "extra": "35234157 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 293232.286,
            "unit": "ns/iter",
            "extra": "454 iterations, 3575.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 310632.812,
            "unit": "ns/iter",
            "extra": "432 iterations, 3375.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 322508.758,
            "unit": "ns/iter",
            "extra": "433 iterations, 3250.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 329069.818,
            "unit": "ns/iter",
            "extra": "373 iterations, 3186.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 297565.048,
            "unit": "ns/iter",
            "extra": "458 iterations, 3523.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 308249.798,
            "unit": "ns/iter",
            "extra": "411 iterations, 3401.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 340259.866,
            "unit": "ns/iter",
            "extra": "397 iterations, 3081.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 446915.743,
            "unit": "ns/iter",
            "extra": "315 iterations, 2334.5MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1299149.16,
            "unit": "ns/iter",
            "extra": "100 iterations, 807.1MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1355970.42,
            "unit": "ns/iter",
            "extra": "100 iterations, 773.3MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2133819.672,
            "unit": "ns/iter",
            "extra": "61 iterations, 491.4MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 2761336,
            "unit": "ns/iter",
            "extra": "47 iterations, 377.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 65828.046,
            "unit": "ns/iter",
            "extra": "2640 iterations, 236.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 127907.038,
            "unit": "ns/iter",
            "extra": "1454 iterations, 246.4MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 140112.092,
            "unit": "ns/iter",
            "extra": "1104 iterations, 87.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 286690.511,
            "unit": "ns/iter",
            "extra": "519 iterations, 85.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 353720.623,
            "unit": "ns/iter",
            "extra": "512 iterations, 95.4MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 60690.628,
            "unit": "ns/iter",
            "extra": "3024 iterations, 103.8MB/s"
          }
        ]
      },
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
          "id": "8d9b68d25b4e273de10fc632f87092bb2e042d7e",
          "message": "Perform colouring without an explicit interference graph",
          "timestamp": "2026-10-04T12:27:54+02:00",
          "tree_id": "5cbd014c61d895a735fe19e1956ed9a08cb0a8cd",
          "url": "https://github.com/sgatev/lucid/commit/8d9b68d25b4e273de10fc632f87092bb2e042d7e"
        },
        "date": 1791109984042,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 1950.481,
            "unit": "ns/iter",
            "extra": "69373 iterations, 918.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 6242.023,
            "unit": "ns/iter",
            "extra": "21390 iterations, 1223.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 12184.212,
            "unit": "ns/iter",
            "extra": "10000 iterations, 1278.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 2879.577,
            "unit": "ns/iter",
            "extra": "47768 iterations, 1174.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 34573.256,
            "unit": "ns/iter",
            "extra": "3866 iterations, 220.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 132543.871,
            "unit": "ns/iter",
            "extra": "1002 iterations, 25.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorDiamonds30",
            "value": 22879.83,
            "unit": "ns/iter",
            "extra": "6116 iterations, 423.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds15",
            "value": 220745.333,
            "unit": "ns/iter",
            "extra": "616 iterations, 38.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds30",
            "value": 478435.942,
            "unit": "ns/iter",
            "extra": "294 iterations, 33.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateBranching40",
            "value": 62287.017,
            "unit": "ns/iter",
            "extra": "2231 iterations, 52.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLoopCarried30",
            "value": 44147.613,
            "unit": "ns/iter",
            "extra": "3184 iterations, 53.4MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf10Functions",
            "value": 621.024,
            "unit": "ns/iter",
            "extra": "197593 iterations, 210.9MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf1000Functions",
            "value": 606.974,
            "unit": "ns/iter",
            "extra": "212123 iterations, 219.1MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 219.457,
            "unit": "ns/iter",
            "extra": "637692 iterations, 241.5MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 4635.17,
            "unit": "ns/iter",
            "extra": "29856 iterations, 1647.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 8847.487,
            "unit": "ns/iter",
            "extra": "20000 iterations, 1759.9MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 8460.054,
            "unit": "ns/iter",
            "extra": "20000 iterations, 732.9MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 15816.933,
            "unit": "ns/iter",
            "extra": "8915 iterations, 776.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenBranches64",
            "value": 21076.873,
            "unit": "ns/iter",
            "extra": "6585 iterations, 582.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenChainedValues512",
            "value": 12653.321,
            "unit": "ns/iter",
            "extra": "10000 iterations, 1230.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 2731.301,
            "unit": "ns/iter",
            "extra": "50589 iterations, 1238.2MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 20.805,
            "unit": "ns/iter",
            "extra": "16908551 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 4.45,
            "unit": "ns/iter",
            "extra": "30778816 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 21.425,
            "unit": "ns/iter",
            "extra": "20653156 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 4.775,
            "unit": "ns/iter",
            "extra": "28799177 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 19.563,
            "unit": "ns/iter",
            "extra": "15460430 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 6.77,
            "unit": "ns/iter",
            "extra": "13619780 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 4.821,
            "unit": "ns/iter",
            "extra": "28082140 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 2.71,
            "unit": "ns/iter",
            "extra": "56098171 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 2.81,
            "unit": "ns/iter",
            "extra": "45591467 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 20.704,
            "unit": "ns/iter",
            "extra": "19246962 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 3.305,
            "unit": "ns/iter",
            "extra": "41479945 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 2.406,
            "unit": "ns/iter",
            "extra": "58245358 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 2.288,
            "unit": "ns/iter",
            "extra": "62224517 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 2.715,
            "unit": "ns/iter",
            "extra": "50753747 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.354,
            "unit": "ns/iter",
            "extra": "385754186 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 1.58,
            "unit": "ns/iter",
            "extra": "88221430 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 3.99,
            "unit": "ns/iter",
            "extra": "35502583 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 284680.584,
            "unit": "ns/iter",
            "extra": "476 iterations, 3683.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 312811.848,
            "unit": "ns/iter",
            "extra": "446 iterations, 3352.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 322377.044,
            "unit": "ns/iter",
            "extra": "428 iterations, 3252.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 318783.266,
            "unit": "ns/iter",
            "extra": "372 iterations, 3289.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 293295.361,
            "unit": "ns/iter",
            "extra": "451 iterations, 3575.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 314719.01,
            "unit": "ns/iter",
            "extra": "402 iterations, 3331.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 333443.234,
            "unit": "ns/iter",
            "extra": "367 iterations, 3144.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 432081.126,
            "unit": "ns/iter",
            "extra": "302 iterations, 2414.7MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1277602.92,
            "unit": "ns/iter",
            "extra": "100 iterations, 820.7MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1345747.5,
            "unit": "ns/iter",
            "extra": "100 iterations, 779.2MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2128528.641,
            "unit": "ns/iter",
            "extra": "64 iterations, 492.6MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 2748628.388,
            "unit": "ns/iter",
            "extra": "49 iterations, 379.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 64441.197,
            "unit": "ns/iter",
            "extra": "2784 iterations, 241.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 126699.104,
            "unit": "ns/iter",
            "extra": "1386 iterations, 248.7MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 143392.743,
            "unit": "ns/iter",
            "extra": "998 iterations, 85.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 284334.146,
            "unit": "ns/iter",
            "extra": "512 iterations, 86.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 348183.185,
            "unit": "ns/iter",
            "extra": "507 iterations, 96.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 59893.07,
            "unit": "ns/iter",
            "extra": "3053 iterations, 105.2MB/s"
          }
        ]
      },
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
          "id": "9d0ec3af33b0a5f30f8b2234e617b0abdfe39bbd",
          "message": "Avoid creating unreachable blocks",
          "timestamp": "2026-10-04T13:30:45+02:00",
          "tree_id": "1164365a17de1bedcf70a2619ac7ab8c24766bcd",
          "url": "https://github.com/sgatev/lucid/commit/9d0ec3af33b0a5f30f8b2234e617b0abdfe39bbd"
        },
        "date": 1791113721161,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 1796.109,
            "unit": "ns/iter",
            "extra": "72302 iterations, 997.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 6460.69,
            "unit": "ns/iter",
            "extra": "22559 iterations, 1181.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 12391.792,
            "unit": "ns/iter",
            "extra": "10000 iterations, 1256.6MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 3170.384,
            "unit": "ns/iter",
            "extra": "46424 iterations, 1066.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 35274.656,
            "unit": "ns/iter",
            "extra": "3760 iterations, 216.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 139600.25,
            "unit": "ns/iter",
            "extra": "968 iterations, 24.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorDiamonds30",
            "value": 23236.777,
            "unit": "ns/iter",
            "extra": "6009 iterations, 417.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds15",
            "value": 225618.622,
            "unit": "ns/iter",
            "extra": "601 iterations, 37.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds30",
            "value": 492529.887,
            "unit": "ns/iter",
            "extra": "283 iterations, 32.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateBranching40",
            "value": 63174.392,
            "unit": "ns/iter",
            "extra": "2152 iterations, 51.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLoopCarried30",
            "value": 46108.517,
            "unit": "ns/iter",
            "extra": "2998 iterations, 51.1MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf10Functions",
            "value": 705.205,
            "unit": "ns/iter",
            "extra": "165271 iterations, 185.8MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf1000Functions",
            "value": 718.237,
            "unit": "ns/iter",
            "extra": "194808 iterations, 185.2MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 270.044,
            "unit": "ns/iter",
            "extra": "506542 iterations, 196.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 4718.835,
            "unit": "ns/iter",
            "extra": "28881 iterations, 1618.0MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 9120.498,
            "unit": "ns/iter",
            "extra": "20000 iterations, 1707.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 8516.962,
            "unit": "ns/iter",
            "extra": "20000 iterations, 728.0MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 15611.831,
            "unit": "ns/iter",
            "extra": "8780 iterations, 786.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenBranches64",
            "value": 21316.871,
            "unit": "ns/iter",
            "extra": "6596 iterations, 576.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenChainedValues512",
            "value": 12878.571,
            "unit": "ns/iter",
            "extra": "10000 iterations, 1209.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 2958.941,
            "unit": "ns/iter",
            "extra": "49881 iterations, 1143.0MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 17.898,
            "unit": "ns/iter",
            "extra": "16370837 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 4.489,
            "unit": "ns/iter",
            "extra": "29002019 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 21.477,
            "unit": "ns/iter",
            "extra": "19706166 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 4.797,
            "unit": "ns/iter",
            "extra": "28115272 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 23.869,
            "unit": "ns/iter",
            "extra": "17205535 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 6.659,
            "unit": "ns/iter",
            "extra": "21584815 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 4.203,
            "unit": "ns/iter",
            "extra": "35611702 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 3.172,
            "unit": "ns/iter",
            "extra": "59047861 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 2.888,
            "unit": "ns/iter",
            "extra": "46961490 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 20.755,
            "unit": "ns/iter",
            "extra": "18655679 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 3.623,
            "unit": "ns/iter",
            "extra": "39451900 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 2.527,
            "unit": "ns/iter",
            "extra": "52342162 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 2.323,
            "unit": "ns/iter",
            "extra": "61473178 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 0,
            "unit": "ns/iter",
            "extra": "1000000000 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 2.747,
            "unit": "ns/iter",
            "extra": "50484561 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.358,
            "unit": "ns/iter",
            "extra": "371046037 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 1.579,
            "unit": "ns/iter",
            "extra": "66823128 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 4.073,
            "unit": "ns/iter",
            "extra": "34010853 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 297519.541,
            "unit": "ns/iter",
            "extra": "484 iterations, 3524.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 309517.713,
            "unit": "ns/iter",
            "extra": "414 iterations, 3387.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 323497.642,
            "unit": "ns/iter",
            "extra": "424 iterations, 3241.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 326471.925,
            "unit": "ns/iter",
            "extra": "371 iterations, 3211.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 297147.487,
            "unit": "ns/iter",
            "extra": "454 iterations, 3528.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 315977.273,
            "unit": "ns/iter",
            "extra": "418 iterations, 3318.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 331723.463,
            "unit": "ns/iter",
            "extra": "369 iterations, 3160.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 446647.64,
            "unit": "ns/iter",
            "extra": "311 iterations, 2335.9MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1289780.42,
            "unit": "ns/iter",
            "extra": "100 iterations, 813.0MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1382505.155,
            "unit": "ns/iter",
            "extra": "97 iterations, 758.5MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2177714.175,
            "unit": "ns/iter",
            "extra": "57 iterations, 481.5MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 2847370.457,
            "unit": "ns/iter",
            "extra": "46 iterations, 366.4MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 66646.968,
            "unit": "ns/iter",
            "extra": "2570 iterations, 233.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 128558.918,
            "unit": "ns/iter",
            "extra": "1425 iterations, 245.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 140014.065,
            "unit": "ns/iter",
            "extra": "1102 iterations, 87.7MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 287541.665,
            "unit": "ns/iter",
            "extra": "499 iterations, 85.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 359029.582,
            "unit": "ns/iter",
            "extra": "500 iterations, 94.0MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 61246.011,
            "unit": "ns/iter",
            "extra": "2977 iterations, 102.9MB/s"
          }
        ]
      },
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
          "id": "430789bdcae16ac197f23f61cd4f62484d1e84af",
          "message": "Fix RemoveMissing hash map and set benchmark",
          "timestamp": "2026-10-04T13:47:49+02:00",
          "tree_id": "7dc949b386d1c648fbd19e4e67a169d5bd5e6759",
          "url": "https://github.com/sgatev/lucid/commit/430789bdcae16ac197f23f61cd4f62484d1e84af"
        },
        "date": 1791114749794,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 1811.849,
            "unit": "ns/iter",
            "extra": "72917 iterations, 988.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 6372.617,
            "unit": "ns/iter",
            "extra": "20000 iterations, 1198.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 12625.179,
            "unit": "ns/iter",
            "extra": "10000 iterations, 1233.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 2942.424,
            "unit": "ns/iter",
            "extra": "44499 iterations, 1149.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 34884.741,
            "unit": "ns/iter",
            "extra": "3931 iterations, 218.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 134852.087,
            "unit": "ns/iter",
            "extra": "1002 iterations, 25.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorDiamonds30",
            "value": 22689.54,
            "unit": "ns/iter",
            "extra": "6139 iterations, 427.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds15",
            "value": 221215.481,
            "unit": "ns/iter",
            "extra": "618 iterations, 38.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds30",
            "value": 485866.955,
            "unit": "ns/iter",
            "extra": "290 iterations, 33.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateBranching40",
            "value": 63951.994,
            "unit": "ns/iter",
            "extra": "2136 iterations, 50.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLoopCarried30",
            "value": 45651.826,
            "unit": "ns/iter",
            "extra": "3080 iterations, 51.6MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf10Functions",
            "value": 682.299,
            "unit": "ns/iter",
            "extra": "166829 iterations, 192.0MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf1000Functions",
            "value": 680.884,
            "unit": "ns/iter",
            "extra": "201171 iterations, 195.3MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 257.453,
            "unit": "ns/iter",
            "extra": "585682 iterations, 205.9MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 4645.899,
            "unit": "ns/iter",
            "extra": "29058 iterations, 1643.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 8847.352,
            "unit": "ns/iter",
            "extra": "20000 iterations, 1760.0MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 8570.094,
            "unit": "ns/iter",
            "extra": "20000 iterations, 723.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 15674.064,
            "unit": "ns/iter",
            "extra": "8308 iterations, 783.5MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenBranches64",
            "value": 21360.982,
            "unit": "ns/iter",
            "extra": "6474 iterations, 574.9MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenChainedValues512",
            "value": 12738.612,
            "unit": "ns/iter",
            "extra": "10000 iterations, 1222.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 2777.481,
            "unit": "ns/iter",
            "extra": "49941 iterations, 1217.7MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 20.671,
            "unit": "ns/iter",
            "extra": "16914255 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 4.473,
            "unit": "ns/iter",
            "extra": "30694181 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 21.382,
            "unit": "ns/iter",
            "extra": "19874599 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 4.784,
            "unit": "ns/iter",
            "extra": "28526066 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 24.968,
            "unit": "ns/iter",
            "extra": "20162623 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 6.079,
            "unit": "ns/iter",
            "extra": "20984260 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 3.727,
            "unit": "ns/iter",
            "extra": "34068441 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 2.597,
            "unit": "ns/iter",
            "extra": "39920626 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 2.345,
            "unit": "ns/iter",
            "extra": "61181003 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 2.802,
            "unit": "ns/iter",
            "extra": "42377691 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 20.27,
            "unit": "ns/iter",
            "extra": "18892538 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 3.431,
            "unit": "ns/iter",
            "extra": "40270873 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 2.473,
            "unit": "ns/iter",
            "extra": "54737383 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 2.336,
            "unit": "ns/iter",
            "extra": "60681584 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 2.247,
            "unit": "ns/iter",
            "extra": "60100900 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 2.715,
            "unit": "ns/iter",
            "extra": "51107331 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.355,
            "unit": "ns/iter",
            "extra": "382184731 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 1.572,
            "unit": "ns/iter",
            "extra": "89392609 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 3.953,
            "unit": "ns/iter",
            "extra": "34204037 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 284787.257,
            "unit": "ns/iter",
            "extra": "463 iterations, 3681.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 310586.283,
            "unit": "ns/iter",
            "extra": "438 iterations, 3376.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 369637.781,
            "unit": "ns/iter",
            "extra": "401 iterations, 2836.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 322195.454,
            "unit": "ns/iter",
            "extra": "372 iterations, 3254.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 290506.111,
            "unit": "ns/iter",
            "extra": "416 iterations, 3609.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 309940.023,
            "unit": "ns/iter",
            "extra": "355 iterations, 3383.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 334661.224,
            "unit": "ns/iter",
            "extra": "398 iterations, 3132.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 435997.381,
            "unit": "ns/iter",
            "extra": "318 iterations, 2393.0MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1276847.09,
            "unit": "ns/iter",
            "extra": "100 iterations, 821.2MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1357531.147,
            "unit": "ns/iter",
            "extra": "95 iterations, 772.4MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2175395.821,
            "unit": "ns/iter",
            "extra": "56 iterations, 482.0MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 2801537.234,
            "unit": "ns/iter",
            "extra": "47 iterations, 372.4MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 65275.651,
            "unit": "ns/iter",
            "extra": "2677 iterations, 238.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 126767.983,
            "unit": "ns/iter",
            "extra": "1462 iterations, 248.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 138327.691,
            "unit": "ns/iter",
            "extra": "1093 iterations, 88.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 279625.083,
            "unit": "ns/iter",
            "extra": "505 iterations, 87.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 342829.868,
            "unit": "ns/iter",
            "extra": "529 iterations, 98.4MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 60466.562,
            "unit": "ns/iter",
            "extra": "3038 iterations, 104.2MB/s"
          }
        ]
      },
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
          "id": "120f7bee49e58f42e177191c0fe9bd58f0b535cb",
          "message": "Track only redefined variables in syntax liveness",
          "timestamp": "2026-10-04T14:15:19+02:00",
          "tree_id": "fa9c062cdb01310cb0446a313e0818268d033e80",
          "url": "https://github.com/sgatev/lucid/commit/120f7bee49e58f42e177191c0fe9bd58f0b535cb"
        },
        "date": 1791116622987,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 1856.764,
            "unit": "ns/iter",
            "extra": "71665 iterations, 964.6MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 6403.718,
            "unit": "ns/iter",
            "extra": "22123 iterations, 1192.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 12473.421,
            "unit": "ns/iter",
            "extra": "10000 iterations, 1248.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 3062.947,
            "unit": "ns/iter",
            "extra": "44672 iterations, 1104.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 35440.649,
            "unit": "ns/iter",
            "extra": "3904 iterations, 215.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 137935.148,
            "unit": "ns/iter",
            "extra": "992 iterations, 24.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorDiamonds30",
            "value": 23249.505,
            "unit": "ns/iter",
            "extra": "5893 iterations, 417.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds15",
            "value": 230402.174,
            "unit": "ns/iter",
            "extra": "598 iterations, 36.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds30",
            "value": 494991.756,
            "unit": "ns/iter",
            "extra": "283 iterations, 32.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateBranching40",
            "value": 64621.355,
            "unit": "ns/iter",
            "extra": "2149 iterations, 50.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLoopCarried30",
            "value": 46290.887,
            "unit": "ns/iter",
            "extra": "2991 iterations, 50.9MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf10Functions",
            "value": 697.425,
            "unit": "ns/iter",
            "extra": "156505 iterations, 187.8MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf1000Functions",
            "value": 697.848,
            "unit": "ns/iter",
            "extra": "198399 iterations, 190.6MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 265.778,
            "unit": "ns/iter",
            "extra": "522737 iterations, 199.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 4798.318,
            "unit": "ns/iter",
            "extra": "27856 iterations, 1591.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 9059.794,
            "unit": "ns/iter",
            "extra": "20000 iterations, 1718.7MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 8701.831,
            "unit": "ns/iter",
            "extra": "20000 iterations, 712.5MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 16301.125,
            "unit": "ns/iter",
            "extra": "8344 iterations, 753.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenBranches64",
            "value": 21908.296,
            "unit": "ns/iter",
            "extra": "6401 iterations, 560.5MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenChainedValues512",
            "value": 13081.037,
            "unit": "ns/iter",
            "extra": "9834 iterations, 1190.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 2853.259,
            "unit": "ns/iter",
            "extra": "48779 iterations, 1185.3MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 17.796,
            "unit": "ns/iter",
            "extra": "16197923 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 4.58,
            "unit": "ns/iter",
            "extra": "30968881 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 21.381,
            "unit": "ns/iter",
            "extra": "19759824 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 4.809,
            "unit": "ns/iter",
            "extra": "28895524 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 24.351,
            "unit": "ns/iter",
            "extra": "19874484 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 5.592,
            "unit": "ns/iter",
            "extra": "22558359 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 3.809,
            "unit": "ns/iter",
            "extra": "28197614 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 2.611,
            "unit": "ns/iter",
            "extra": "40902280 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 2.478,
            "unit": "ns/iter",
            "extra": "60499126 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 2.881,
            "unit": "ns/iter",
            "extra": "46437066 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 20.372,
            "unit": "ns/iter",
            "extra": "18231440 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 3.388,
            "unit": "ns/iter",
            "extra": "30023055 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 2.489,
            "unit": "ns/iter",
            "extra": "54493261 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 2.318,
            "unit": "ns/iter",
            "extra": "60291760 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 2.288,
            "unit": "ns/iter",
            "extra": "61704589 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 2.762,
            "unit": "ns/iter",
            "extra": "48826558 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.36,
            "unit": "ns/iter",
            "extra": "387281487 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 1.619,
            "unit": "ns/iter",
            "extra": "84604925 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 4.183,
            "unit": "ns/iter",
            "extra": "35338284 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 296690.364,
            "unit": "ns/iter",
            "extra": "473 iterations, 3534.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 330025.202,
            "unit": "ns/iter",
            "extra": "415 iterations, 3177.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 349575.97,
            "unit": "ns/iter",
            "extra": "396 iterations, 2999.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 336779.536,
            "unit": "ns/iter",
            "extra": "371 iterations, 3113.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 309613.255,
            "unit": "ns/iter",
            "extra": "447 iterations, 3386.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 330935.533,
            "unit": "ns/iter",
            "extra": "413 iterations, 3168.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 358139.87,
            "unit": "ns/iter",
            "extra": "339 iterations, 2927.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 443588.162,
            "unit": "ns/iter",
            "extra": "302 iterations, 2352.0MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1306615.84,
            "unit": "ns/iter",
            "extra": "100 iterations, 802.5MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1384210.83,
            "unit": "ns/iter",
            "extra": "100 iterations, 757.5MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2195135.918,
            "unit": "ns/iter",
            "extra": "61 iterations, 477.7MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 2834346.638,
            "unit": "ns/iter",
            "extra": "47 iterations, 368.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 58832.012,
            "unit": "ns/iter",
            "extra": "2869 iterations, 264.7MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 114991.239,
            "unit": "ns/iter",
            "extra": "1636 iterations, 274.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 146314.401,
            "unit": "ns/iter",
            "extra": "1063 iterations, 83.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 291040.263,
            "unit": "ns/iter",
            "extra": "505 iterations, 84.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 357480.042,
            "unit": "ns/iter",
            "extra": "499 iterations, 94.4MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 63143.994,
            "unit": "ns/iter",
            "extra": "2889 iterations, 99.8MB/s"
          }
        ]
      },
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
          "id": "dbbd108dff43ca97dcdd9b5ea9a2eb9fffeddd5d",
          "message": "Keep the dataflow worklist as a bitset over the order it is worked in",
          "timestamp": "2026-10-04T14:48:10+02:00",
          "tree_id": "99b3cf271c312b39a2b7b2cc28576640470db754",
          "url": "https://github.com/sgatev/lucid/commit/dbbd108dff43ca97dcdd9b5ea9a2eb9fffeddd5d"
        },
        "date": 1791118371255,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 1777.391,
            "unit": "ns/iter",
            "extra": "73687 iterations, 1007.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 6210.073,
            "unit": "ns/iter",
            "extra": "20000 iterations, 1229.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 12116.042,
            "unit": "ns/iter",
            "extra": "10000 iterations, 1285.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 2928.436,
            "unit": "ns/iter",
            "extra": "47140 iterations, 1154.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 34681.1,
            "unit": "ns/iter",
            "extra": "3932 iterations, 220.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 132844.727,
            "unit": "ns/iter",
            "extra": "1013 iterations, 25.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorDiamonds30",
            "value": 22217.245,
            "unit": "ns/iter",
            "extra": "6284 iterations, 436.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds15",
            "value": 214870.028,
            "unit": "ns/iter",
            "extra": "637 iterations, 39.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds30",
            "value": 462577.637,
            "unit": "ns/iter",
            "extra": "300 iterations, 35.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateBranching40",
            "value": 62564.367,
            "unit": "ns/iter",
            "extra": "2076 iterations, 51.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLoopCarried30",
            "value": 43483.887,
            "unit": "ns/iter",
            "extra": "3173 iterations, 54.2MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf10Functions",
            "value": 696.818,
            "unit": "ns/iter",
            "extra": "161484 iterations, 188.0MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf1000Functions",
            "value": 705.67,
            "unit": "ns/iter",
            "extra": "198951 iterations, 188.5MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 267.093,
            "unit": "ns/iter",
            "extra": "491450 iterations, 198.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 4637.443,
            "unit": "ns/iter",
            "extra": "29199 iterations, 1646.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 8839.208,
            "unit": "ns/iter",
            "extra": "20000 iterations, 1761.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 8406.785,
            "unit": "ns/iter",
            "extra": "20000 iterations, 737.5MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 15826.536,
            "unit": "ns/iter",
            "extra": "8913 iterations, 775.9MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenBranches64",
            "value": 21428.311,
            "unit": "ns/iter",
            "extra": "5890 iterations, 573.1MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenChainedValues512",
            "value": 12650.1,
            "unit": "ns/iter",
            "extra": "10000 iterations, 1230.9MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 2737.145,
            "unit": "ns/iter",
            "extra": "53053 iterations, 1235.6MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 20.619,
            "unit": "ns/iter",
            "extra": "16973811 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 4.429,
            "unit": "ns/iter",
            "extra": "30132637 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 21.195,
            "unit": "ns/iter",
            "extra": "20508190 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 4.752,
            "unit": "ns/iter",
            "extra": "27538726 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 24.991,
            "unit": "ns/iter",
            "extra": "20809980 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 5.079,
            "unit": "ns/iter",
            "extra": "18991311 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 3.708,
            "unit": "ns/iter",
            "extra": "31628589 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 2.833,
            "unit": "ns/iter",
            "extra": "57867189 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 2.351,
            "unit": "ns/iter",
            "extra": "60847501 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 2.845,
            "unit": "ns/iter",
            "extra": "62603630 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 20.104,
            "unit": "ns/iter",
            "extra": "18287006 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 3.38,
            "unit": "ns/iter",
            "extra": "40506329 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 2.416,
            "unit": "ns/iter",
            "extra": "59113283 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 2.276,
            "unit": "ns/iter",
            "extra": "62801390 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 2.208,
            "unit": "ns/iter",
            "extra": "63831019 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 2.676,
            "unit": "ns/iter",
            "extra": "51869468 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.26,
            "unit": "ns/iter",
            "extra": "523208012 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 0.465,
            "unit": "ns/iter",
            "extra": "297814520 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 1.161,
            "unit": "ns/iter",
            "extra": "100000000 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 281815.75,
            "unit": "ns/iter",
            "extra": "500 iterations, 3720.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 306770.232,
            "unit": "ns/iter",
            "extra": "449 iterations, 3418.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 311577.945,
            "unit": "ns/iter",
            "extra": "379 iterations, 3365.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 312527.526,
            "unit": "ns/iter",
            "extra": "386 iterations, 3355.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 288205.526,
            "unit": "ns/iter",
            "extra": "460 iterations, 3638.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 301777.022,
            "unit": "ns/iter",
            "extra": "404 iterations, 3474.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 338815.901,
            "unit": "ns/iter",
            "extra": "435 iterations, 3094.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 441910.833,
            "unit": "ns/iter",
            "extra": "300 iterations, 2360.9MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1253947.5,
            "unit": "ns/iter",
            "extra": "100 iterations, 836.2MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1339517.92,
            "unit": "ns/iter",
            "extra": "100 iterations, 782.8MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2101402.778,
            "unit": "ns/iter",
            "extra": "63 iterations, 499.0MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 2729778.918,
            "unit": "ns/iter",
            "extra": "49 iterations, 382.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 56534.209,
            "unit": "ns/iter",
            "extra": "3179 iterations, 275.4MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 111558.468,
            "unit": "ns/iter",
            "extra": "1669 iterations, 282.5MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 120023.54,
            "unit": "ns/iter",
            "extra": "1301 iterations, 102.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 241816.381,
            "unit": "ns/iter",
            "extra": "585 iterations, 101.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 325151.06,
            "unit": "ns/iter",
            "extra": "550 iterations, 103.7MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 60675.815,
            "unit": "ns/iter",
            "extra": "2965 iterations, 103.9MB/s"
          }
        ]
      },
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
          "id": "abae48f04159e1507c1797676615033b608335cf",
          "message": "Hold what is live where as words of register bits",
          "timestamp": "2026-10-04T16:41:42+02:00",
          "tree_id": "66054ccc6e8e421e3a85ae6326d7902dfa3e5abc",
          "url": "https://github.com/sgatev/lucid/commit/abae48f04159e1507c1797676615033b608335cf"
        },
        "date": 1791125194656,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 1873.617,
            "unit": "ns/iter",
            "extra": "71580 iterations, 955.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 6534.559,
            "unit": "ns/iter",
            "extra": "21976 iterations, 1168.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 12862.397,
            "unit": "ns/iter",
            "extra": "9723 iterations, 1210.6MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 2973.705,
            "unit": "ns/iter",
            "extra": "44798 iterations, 1137.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 33295.32,
            "unit": "ns/iter",
            "extra": "4094 iterations, 229.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 135292.651,
            "unit": "ns/iter",
            "extra": "1015 iterations, 25.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorDiamonds30",
            "value": 17701.907,
            "unit": "ns/iter",
            "extra": "7554 iterations, 547.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds15",
            "value": 195783.795,
            "unit": "ns/iter",
            "extra": "704 iterations, 43.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds30",
            "value": 414927.424,
            "unit": "ns/iter",
            "extra": "337 iterations, 39.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateBranching40",
            "value": 52962.972,
            "unit": "ns/iter",
            "extra": "2623 iterations, 61.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLoopCarried30",
            "value": 41297.565,
            "unit": "ns/iter",
            "extra": "3320 iterations, 57.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateHeldLong500",
            "value": 917416.04,
            "unit": "ns/iter",
            "extra": "200 iterations, 61.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateHeldLong2000",
            "value": 3699575.971,
            "unit": "ns/iter",
            "extra": "34 iterations, 62.6MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf10Functions",
            "value": 721.462,
            "unit": "ns/iter",
            "extra": "163694 iterations, 181.6MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf1000Functions",
            "value": 727.741,
            "unit": "ns/iter",
            "extra": "189539 iterations, 182.8MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 285.686,
            "unit": "ns/iter",
            "extra": "480267 iterations, 185.5MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 4719.335,
            "unit": "ns/iter",
            "extra": "28541 iterations, 1617.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 9009.621,
            "unit": "ns/iter",
            "extra": "20000 iterations, 1728.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 8631.671,
            "unit": "ns/iter",
            "extra": "20000 iterations, 718.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 16078.809,
            "unit": "ns/iter",
            "extra": "8694 iterations, 763.7MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenBranches64",
            "value": 21609.369,
            "unit": "ns/iter",
            "extra": "6451 iterations, 568.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenChainedValues512",
            "value": 13075.743,
            "unit": "ns/iter",
            "extra": "9431 iterations, 1190.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 2782.274,
            "unit": "ns/iter",
            "extra": "49571 iterations, 1215.6MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 17.967,
            "unit": "ns/iter",
            "extra": "16649982 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 4.564,
            "unit": "ns/iter",
            "extra": "30194102 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 21.563,
            "unit": "ns/iter",
            "extra": "19714258 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 4.862,
            "unit": "ns/iter",
            "extra": "28161931 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 25.323,
            "unit": "ns/iter",
            "extra": "19128730 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 5.078,
            "unit": "ns/iter",
            "extra": "27467358 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 3.742,
            "unit": "ns/iter",
            "extra": "36689630 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 2.705,
            "unit": "ns/iter",
            "extra": "50994074 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 2.645,
            "unit": "ns/iter",
            "extra": "59622038 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 2.869,
            "unit": "ns/iter",
            "extra": "37442756 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 20.424,
            "unit": "ns/iter",
            "extra": "19189035 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 3.47,
            "unit": "ns/iter",
            "extra": "40362792 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 2.524,
            "unit": "ns/iter",
            "extra": "52797812 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 2.327,
            "unit": "ns/iter",
            "extra": "59586106 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 2.279,
            "unit": "ns/iter",
            "extra": "56939501 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 2.788,
            "unit": "ns/iter",
            "extra": "48362714 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.267,
            "unit": "ns/iter",
            "extra": "525112413 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 0.476,
            "unit": "ns/iter",
            "extra": "294224532 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 1.238,
            "unit": "ns/iter",
            "extra": "100000000 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 294513.2,
            "unit": "ns/iter",
            "extra": "464 iterations, 3560.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 325605.11,
            "unit": "ns/iter",
            "extra": "419 iterations, 3220.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 353699.637,
            "unit": "ns/iter",
            "extra": "369 iterations, 2964.3MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 335226.36,
            "unit": "ns/iter",
            "extra": "356 iterations, 3127.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 313141.482,
            "unit": "ns/iter",
            "extra": "407 iterations, 3348.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 336947.301,
            "unit": "ns/iter",
            "extra": "389 iterations, 3111.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 345322.139,
            "unit": "ns/iter",
            "extra": "402 iterations, 3036.0MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 458690.06,
            "unit": "ns/iter",
            "extra": "301 iterations, 2274.6MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1307090.42,
            "unit": "ns/iter",
            "extra": "100 iterations, 802.2MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1386769.364,
            "unit": "ns/iter",
            "extra": "99 iterations, 756.1MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2157911.885,
            "unit": "ns/iter",
            "extra": "61 iterations, 485.9MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 2821296.196,
            "unit": "ns/iter",
            "extra": "46 iterations, 369.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 58938.209,
            "unit": "ns/iter",
            "extra": "3056 iterations, 264.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 115367.15,
            "unit": "ns/iter",
            "extra": "1619 iterations, 273.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 125409.361,
            "unit": "ns/iter",
            "extra": "1249 iterations, 97.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 250340.327,
            "unit": "ns/iter",
            "extra": "572 iterations, 97.7MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 344822.471,
            "unit": "ns/iter",
            "extra": "514 iterations, 97.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 62665.893,
            "unit": "ns/iter",
            "extra": "2909 iterations, 100.6MB/s"
          }
        ]
      },
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
          "id": "56f9bbe31ee154434a71d29b26ad89a0cc0507df",
          "message": "Add an end-to-end compilation benchmark",
          "timestamp": "2026-10-04T16:49:05+02:00",
          "tree_id": "26b2f597fbd87cd832380e8bc5bd8a50098fba4f",
          "url": "https://github.com/sgatev/lucid/commit/56f9bbe31ee154434a71d29b26ad89a0cc0507df"
        },
        "date": 1791125628533,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 1961.11,
            "unit": "ns/iter",
            "extra": "67685 iterations, 913.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 6690.744,
            "unit": "ns/iter",
            "extra": "20000 iterations, 1141.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 12986.125,
            "unit": "ns/iter",
            "extra": "10000 iterations, 1199.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 3077.521,
            "unit": "ns/iter",
            "extra": "45545 iterations, 1098.9MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 32729.087,
            "unit": "ns/iter",
            "extra": "3917 iterations, 233.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 136404.332,
            "unit": "ns/iter",
            "extra": "1010 iterations, 24.8MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorDiamonds30",
            "value": 17823.353,
            "unit": "ns/iter",
            "extra": "7427 iterations, 544.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds15",
            "value": 200475.801,
            "unit": "ns/iter",
            "extra": "687 iterations, 42.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds30",
            "value": 423323.358,
            "unit": "ns/iter",
            "extra": "330 iterations, 38.3MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateBranching40",
            "value": 54051.831,
            "unit": "ns/iter",
            "extra": "2599 iterations, 60.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLoopCarried30",
            "value": 42365.563,
            "unit": "ns/iter",
            "extra": "3276 iterations, 55.6MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateHeldLong500",
            "value": 944517.085,
            "unit": "ns/iter",
            "extra": "200 iterations, 59.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateHeldLong2000",
            "value": 3765469.697,
            "unit": "ns/iter",
            "extra": "33 iterations, 61.5MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf10Functions",
            "value": 701.187,
            "unit": "ns/iter",
            "extra": "148770 iterations, 186.8MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf1000Functions",
            "value": 732.605,
            "unit": "ns/iter",
            "extra": "194938 iterations, 181.5MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 254.024,
            "unit": "ns/iter",
            "extra": "547489 iterations, 208.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 4779.563,
            "unit": "ns/iter",
            "extra": "23578 iterations, 1597.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 9053.979,
            "unit": "ns/iter",
            "extra": "20000 iterations, 1719.8MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 8837.273,
            "unit": "ns/iter",
            "extra": "20000 iterations, 701.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 16469.511,
            "unit": "ns/iter",
            "extra": "8477 iterations, 745.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenBranches64",
            "value": 22205.566,
            "unit": "ns/iter",
            "extra": "6233 iterations, 553.0MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenChainedValues512",
            "value": 13111.733,
            "unit": "ns/iter",
            "extra": "10000 iterations, 1187.6MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 2857.583,
            "unit": "ns/iter",
            "extra": "49634 iterations, 1183.5MB/s"
          },
          {
            "name": "lucid/compiler/compiler_bench/Examples",
            "value": 398798.783,
            "unit": "ns/iter",
            "extra": "322 iterations, 17.2MB/s"
          },
          {
            "name": "lucid/compiler/compiler_bench/ExampleFunctions",
            "value": 41570805.667,
            "unit": "ns/iter",
            "extra": "3 iterations, 25.2MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 18.422,
            "unit": "ns/iter",
            "extra": "15366533 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 6.082,
            "unit": "ns/iter",
            "extra": "27910452 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 19.832,
            "unit": "ns/iter",
            "extra": "15937766 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 5.197,
            "unit": "ns/iter",
            "extra": "25759960 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 22.666,
            "unit": "ns/iter",
            "extra": "19144864 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 5.217,
            "unit": "ns/iter",
            "extra": "24983082 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 3.905,
            "unit": "ns/iter",
            "extra": "35133586 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 2.697,
            "unit": "ns/iter",
            "extra": "54734708 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 2.419,
            "unit": "ns/iter",
            "extra": "59760947 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 2.885,
            "unit": "ns/iter",
            "extra": "39463955 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 21.212,
            "unit": "ns/iter",
            "extra": "17654289 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 3.564,
            "unit": "ns/iter",
            "extra": "34604573 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 2.637,
            "unit": "ns/iter",
            "extra": "52621687 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 2.341,
            "unit": "ns/iter",
            "extra": "52389503 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 2.317,
            "unit": "ns/iter",
            "extra": "61602772 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 2.878,
            "unit": "ns/iter",
            "extra": "47850307 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.269,
            "unit": "ns/iter",
            "extra": "501429665 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 0.478,
            "unit": "ns/iter",
            "extra": "297937848 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 1.228,
            "unit": "ns/iter",
            "extra": "94337016 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 314646.859,
            "unit": "ns/iter",
            "extra": "427 iterations, 3332.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 338079.36,
            "unit": "ns/iter",
            "extra": "367 iterations, 3101.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 340197.766,
            "unit": "ns/iter",
            "extra": "414 iterations, 3081.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 336021.301,
            "unit": "ns/iter",
            "extra": "356 iterations, 3120.4MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 324898.158,
            "unit": "ns/iter",
            "extra": "430 iterations, 3227.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 334964.962,
            "unit": "ns/iter",
            "extra": "396 iterations, 3130.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 381627.722,
            "unit": "ns/iter",
            "extra": "352 iterations, 2747.2MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 496593.896,
            "unit": "ns/iter",
            "extra": "288 iterations, 2101.0MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1311868.33,
            "unit": "ns/iter",
            "extra": "100 iterations, 799.3MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1395274.235,
            "unit": "ns/iter",
            "extra": "98 iterations, 751.5MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2204940.855,
            "unit": "ns/iter",
            "extra": "62 iterations, 475.5MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 2864836.894,
            "unit": "ns/iter",
            "extra": "47 iterations, 364.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 60360.862,
            "unit": "ns/iter",
            "extra": "3009 iterations, 258.0MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 118639.345,
            "unit": "ns/iter",
            "extra": "1554 iterations, 265.6MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 129217.886,
            "unit": "ns/iter",
            "extra": "1217 iterations, 95.0MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 259981.679,
            "unit": "ns/iter",
            "extra": "564 iterations, 94.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 354824.612,
            "unit": "ns/iter",
            "extra": "516 iterations, 95.1MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 64108.931,
            "unit": "ns/iter",
            "extra": "2777 iterations, 98.3MB/s"
          }
        ]
      },
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
          "id": "17ca6324ecd8145f881d4ef93c2b108df2a19453",
          "message": "Update README",
          "timestamp": "2026-10-04T17:12:16+02:00",
          "tree_id": "6cf00dc98dd65c1a5b337d8841334e1e9cc5a15b",
          "url": "https://github.com/sgatev/lucid/commit/17ca6324ecd8145f881d4ef93c2b108df2a19453"
        },
        "date": 1791127017099,
        "tool": "customSmallerIsBetter",
        "benches": [
          {
            "name": "lucid/am/reg_bench/ColorChain64",
            "value": 1971.406,
            "unit": "ns/iter",
            "extra": "67607 iterations, 908.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain256",
            "value": 6683.326,
            "unit": "ns/iter",
            "extra": "20437 iterations, 1142.4MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorChain512",
            "value": 13198.804,
            "unit": "ns/iter",
            "extra": "10000 iterations, 1179.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorLive64",
            "value": 3137.153,
            "unit": "ns/iter",
            "extra": "44054 iterations, 1078.0MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateChain256",
            "value": 32458.85,
            "unit": "ns/iter",
            "extra": "4111 iterations, 235.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLive64",
            "value": 134194.471,
            "unit": "ns/iter",
            "extra": "1031 iterations, 25.2MB/s"
          },
          {
            "name": "lucid/am/reg_bench/ColorDiamonds30",
            "value": 18166.834,
            "unit": "ns/iter",
            "extra": "7455 iterations, 533.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds15",
            "value": 198146.879,
            "unit": "ns/iter",
            "extra": "697 iterations, 42.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateDiamonds30",
            "value": 419525.746,
            "unit": "ns/iter",
            "extra": "335 iterations, 38.7MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateBranching40",
            "value": 53632.403,
            "unit": "ns/iter",
            "extra": "2589 iterations, 60.6MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateLoopCarried30",
            "value": 42044.366,
            "unit": "ns/iter",
            "extra": "3303 iterations, 56.1MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateHeldLong500",
            "value": 929129.375,
            "unit": "ns/iter",
            "extra": "200 iterations, 60.5MB/s"
          },
          {
            "name": "lucid/am/reg_bench/AllocateHeldLong2000",
            "value": 3727125,
            "unit": "ns/iter",
            "extra": "34 iterations, 62.2MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf10Functions",
            "value": 701.113,
            "unit": "ns/iter",
            "extra": "156660 iterations, 186.8MB/s"
          },
          {
            "name": "lucid/am/translator_bench/LastOf1000Functions",
            "value": 709.003,
            "unit": "ns/iter",
            "extra": "196190 iterations, 187.6MB/s"
          },
          {
            "name": "lucid/am/translator_bench/Function",
            "value": 297.055,
            "unit": "ns/iter",
            "extra": "416857 iterations, 178.4MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues256",
            "value": 4837.482,
            "unit": "ns/iter",
            "extra": "27737 iterations, 1578.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/ChainedValues512",
            "value": 9128.542,
            "unit": "ns/iter",
            "extra": "20000 iterations, 1705.7MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches32",
            "value": 8853.815,
            "unit": "ns/iter",
            "extra": "20000 iterations, 700.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/Branches64",
            "value": 16612.538,
            "unit": "ns/iter",
            "extra": "8349 iterations, 739.2MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenBranches64",
            "value": 22356.126,
            "unit": "ns/iter",
            "extra": "6157 iterations, 549.3MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/WrittenChainedValues512",
            "value": 13106.65,
            "unit": "ns/iter",
            "extra": "10000 iterations, 1188.0MB/s"
          },
          {
            "name": "lucid/arm64/translator_bench/SpilledValues64",
            "value": 2853.792,
            "unit": "ns/iter",
            "extra": "47978 iterations, 1185.1MB/s"
          },
          {
            "name": "lucid/compiler/compiler_bench/Examples",
            "value": 397452.966,
            "unit": "ns/iter",
            "extra": "326 iterations, 17.3MB/s"
          },
          {
            "name": "lucid/compiler/compiler_bench/ExampleFunctions",
            "value": 41218555.667,
            "unit": "ns/iter",
            "extra": "3 iterations, 25.4MB/s"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceUnique",
            "value": 20.224,
            "unit": "ns/iter",
            "extra": "14647734 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/EmplaceDuplicate",
            "value": 6.53,
            "unit": "ns/iter",
            "extra": "20723728 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertUnique",
            "value": 22.748,
            "unit": "ns/iter",
            "extra": "17355551 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/InsertDuplicate",
            "value": 5.845,
            "unit": "ns/iter",
            "extra": "24894053 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetUnique",
            "value": 21.731,
            "unit": "ns/iter",
            "extra": "17125208 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/SetDuplicate",
            "value": 5.233,
            "unit": "ns/iter",
            "extra": "25655709 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindPresent",
            "value": 3.906,
            "unit": "ns/iter",
            "extra": "33573141 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/FindMissing",
            "value": 2.696,
            "unit": "ns/iter",
            "extra": "54998132 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/RemoveMissing",
            "value": 2.439,
            "unit": "ns/iter",
            "extra": "58077212 iterations"
          },
          {
            "name": "lucid/core/container/hash_map_bench/Random",
            "value": 2.904,
            "unit": "ns/iter",
            "extra": "42463385 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertUnique",
            "value": 20.731,
            "unit": "ns/iter",
            "extra": "17939518 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/InsertDuplicate",
            "value": 3.568,
            "unit": "ns/iter",
            "extra": "38821038 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindPresent",
            "value": 2.661,
            "unit": "ns/iter",
            "extra": "51583579 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/FindMissing",
            "value": 2.364,
            "unit": "ns/iter",
            "extra": "57398620 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/RemoveMissing",
            "value": 2.321,
            "unit": "ns/iter",
            "extra": "61423713 iterations"
          },
          {
            "name": "lucid/core/container/hash_set_bench/Random",
            "value": 2.838,
            "unit": "ns/iter",
            "extra": "47590719 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Push",
            "value": 0.269,
            "unit": "ns/iter",
            "extra": "500792181 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/Pop",
            "value": 0.471,
            "unit": "ns/iter",
            "extra": "289804569 iterations"
          },
          {
            "name": "lucid/core/dataflow/worklist_bench/PushPop",
            "value": 1.189,
            "unit": "ns/iter",
            "extra": "100000000 iterations"
          },
          {
            "name": "lucid/syntax/lexer_bench/Function",
            "value": 309155.416,
            "unit": "ns/iter",
            "extra": "437 iterations, 3391.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Tuple",
            "value": 327615.752,
            "unit": "ns/iter",
            "extra": "419 iterations, 3200.5MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Lambda",
            "value": 344582.944,
            "unit": "ns/iter",
            "extra": "321 iterations, 3042.7MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Union",
            "value": 358639.913,
            "unit": "ns/iter",
            "extra": "366 iterations, 2923.6MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Comment",
            "value": 322239.242,
            "unit": "ns/iter",
            "extra": "364 iterations, 3253.9MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Number",
            "value": 351289.801,
            "unit": "ns/iter",
            "extra": "402 iterations, 2984.8MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Branches",
            "value": 397568.575,
            "unit": "ns/iter",
            "extra": "367 iterations, 2637.1MB/s"
          },
          {
            "name": "lucid/syntax/lexer_bench/Examples",
            "value": 499807.643,
            "unit": "ns/iter",
            "extra": "266 iterations, 2087.5MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Function",
            "value": 1319160,
            "unit": "ns/iter",
            "extra": "100 iterations, 794.9MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Comment",
            "value": 1397095.545,
            "unit": "ns/iter",
            "extra": "99 iterations, 750.5MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Branches",
            "value": 2210581.967,
            "unit": "ns/iter",
            "extra": "61 iterations, 474.3MB/s"
          },
          {
            "name": "lucid/syntax/parser_bench/Examples",
            "value": 2867743.787,
            "unit": "ns/iter",
            "extra": "47 iterations, 363.8MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues512",
            "value": 60526.965,
            "unit": "ns/iter",
            "extra": "3041 iterations, 257.3MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/ChainedValues1024",
            "value": 118322.711,
            "unit": "ns/iter",
            "extra": "1624 iterations, 266.4MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches64",
            "value": 129333.847,
            "unit": "ns/iter",
            "extra": "1217 iterations, 94.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/Branches128",
            "value": 259686.68,
            "unit": "ns/iter",
            "extra": "560 iterations, 94.2MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/BranchesWideVars",
            "value": 351704.109,
            "unit": "ns/iter",
            "extra": "513 iterations, 95.9MB/s"
          },
          {
            "name": "lucid/syntax/ssa_bench/LoopAssignments",
            "value": 64677.295,
            "unit": "ns/iter",
            "extra": "2858 iterations, 97.4MB/s"
          }
        ]
      }
    ]
  }
}