window.BENCHMARK_DATA = {
  "lastUpdate": 1790369417684,
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
      }
    ]
  }
}