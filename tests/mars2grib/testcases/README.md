# mars2grib test cases: tools cheat sheet

Test cases are JSONL records `{"mars":…,"misc":…,"opt":…,"out":…}`, stored as plain/zstd JSONL or as an
eckit block file (`.ekbf`: zstd blocks labelled e.g. class/stream/type/levtype/timeShape).

Tools: `eckit-blockfile` (any block file), `mars2grib-testcases`, `mars2grib-regenerate-tests`,
`mars2grib-run-tests`, `mars2grib-run-tests-scalar`. Every tool accepts `--help`.

Selections: `--select key=v1/v2,key2=a/to/b` (ranges on numeric keys, `missing` for absent values) and
`--blocks 0,3,5-7`; both can be combined. `eckit-blockfile` calls the former `--filter`.

## Inspect

### 1. Summary, blocks as CSV, counts per label
```sh
eckit-blockfile info  tests.ekbf --summary
eckit-blockfile list  tests.ekbf          # block,size,startOffset,endOffset,records,class,stream,…
eckit-blockfile stats tests.ekbf
```

### 2. Check integrity
```sh
eckit-blockfile verify tests.ekbf
```

### 3. Print tests
```sh
mars2grib-testcases get tests.ekbf --index 42                       # one test, pretty-printed
mars2grib-testcases cat tests.ekbf --select stream=enfo --pretty | less
eckit-blockfile cat tests.ekbf --blocks 3,7-9 > some.jsonl          # raw JSONL
```

## Create and reshape

### 4. JSONL (plain or .zst) to block file
```sh
eckit-blockfile convert tests.jsonl.zst --output tests.ekbf --level 22 \
    --keys class=mars.class,stream=mars.stream,type=mars.type      # or --config keys.yaml
```

### 5. Relabel / re-block without regenerating
```sh
# timeShape = shape type of the product time spec; absent keys are labelled "missing"
mars2grib-testcases relabel tests.ekbf --output tests2.ekbf \
    --keys class,stream,type,levtype,timeShape --records-per-block 100 --zstd-level 22
```

### 6. Extract a subset (blocks are copied, not recompressed)
```sh
eckit-blockfile select tests.ekbf --filter "stream=enfo,levtype=missing" --output enfo.ekbf
```

### 7. Remove keys from every record (only the edited section is rewritten)
```sh
mars2grib-testcases remove-keys tests.ekbf --output fixed.ekbf --remove misc.shapeOfTheEarth
```

## Regenerate expected outputs

### 8. Regenerate everything with the current code
```sh
mars2grib-regenerate-tests --input tests.ekbf --output new.ekbf --num-threads 32 \
    --keys class,stream,type,levtype,timeShape --records-per-block 100 --zstd-level 22 \
    --failed-records failed.jsonl
```

### 9. Merge an old file with new JSONL cases, drop duplicates and unwanted requests
```sh
mars2grib-regenerate-tests --input old.ekbf,new-cases.jsonl --output merged.ekbf --num-threads 32 \
    --deduplicate --exclude "stream=enfo,type=pf;class=rd"         # or --exclude-file rules.txt
```

### 10. Regenerate in independent processes (avoids thread contention), then merge
```sh
N=$(eckit-blockfile list tests.ekbf --no-header | wc -l)
mkdir -p parts
seq 0 $((N-1)) | xargs -P 32 -I{} mars2grib-regenerate-tests --input tests.ekbf --blocks {} \
    --output parts/p{}.ekbf --num-threads 3 --keys class,stream,type,levtype,timeShape
eckit-blockfile convert parts/p*.ekbf --output new.ekbf --level 22
```

## Compare

### 11. What changed between two test files (exit status non-zero if they differ)
```sh
mars2grib-testcases diff old.ekbf new.ekbf --details 20
```

## Run

### 12. All tests, multithreaded, with timings
```sh
mars2grib-run-tests --test-cases tests.ekbf --num-threads 32 --timings
```

### 13. One block in decode mode, one report per failure
```sh
mars2grib-run-tests-scalar --test-cases tests.ekbf --blocks 68 --mode decode --decode-mars-only \
    --failure-reports --failure-reports-dir reports \
    --failure-reports-schema '{status}/{class}-{stream}-{type}/{levtype}-{param}-{hash}.txt'
```

### 14. Keep the failed records and rerun only those
```sh
mars2grib-run-tests --test-cases tests.ekbf --num-threads 32 --failed-tests failed   # failed.<thread>.txt
mars2grib-run-tests-scalar --test-cases failed.3.txt --mode roundtrip
```

### 15. Standard scripts (build tree, no arguments)
```sh
cd <build>/tests/mars2grib/testcases
./rerun-all-tests.sh         # mars2grib-run-tests, encode + decode; self-contained reports in failure-reports/
./regenerate-all-tests.sh    # mars2grib-regenerate-tests -> regenerated/mars2grib-testcases-<md5>.ekbf
./upload-test-file.sh        # upload it to Nexus (asks for username/password), then check the download
```

### 16. Corpus ctests (two per block: encode and decode), off by default
```sh
# the test file is downloaded at configure time (-DMARS2GRIB_TESTCASES_VERSION=<md5> for another one)
cmake <build> -DENABLE_MARS2GRIB_ROUNDTRIP_TESTCASES=ON
make mars2grib_testcases_ctests
ctest -N -L mars2grib_corpus | tail -1
ctest -j 32 -L mars2grib_corpus-decode
ctest -L "^block=42$" --output-on-failure
```
