window.BENCHMARK_DATA = {
  "lastUpdate": 1789935041630,
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
      }
    ]
  }
}