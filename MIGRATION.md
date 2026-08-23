# Migration guide from v1.x to v2.0.0 / v1.x から v2.0.0 への移行ガイド

In v2.0.0, all user-facing macros were renamed to avoid name collisions: directive macros carry the `SOLOMON_` prefix and clause tokens carry the `SOLOMON_CLAUSE_` prefix. Codes written for v1.x keep working by defining `SOLOMON_WITH_SHORT_NAMES`. This file lists the complete correspondence.

v2.0.0 では名前衝突を避けるため，ユーザー向けマクロを全て改名しました（指示文マクロは `SOLOMON_` 接頭辞，引数トークンは `SOLOMON_CLAUSE_` 接頭辞）．v1.x 向けのコードは `SOLOMON_WITH_SHORT_NAMES` を定義すればそのまま動作します．本ファイルは完全な対応表です．

A migration script is bundled / 移行スクリプトを同梱しています:

```sh
misc/migrate/solomon_migrate_v1_to_v2.sh <file|directory> ...          # dry-run (show diff)
misc/migrate/solomon_migrate_v1_to_v2.sh --apply <file|directory> ...  # rewrite in place (*.v1.bak backups)
```

To verify a migration, compare the preprocessed sources before and after (C/C++: compiler `-E` output; Fortran: the spp output); they must be identical.
移行の検証には，変換前後の前処理済みソース（C/C++ はコンパイラの `-E` 出力，Fortran は spp 出力）を diff 比較してください（完全一致するはずです）．

## Directive macros / 指示文マクロ

| v1.x | v2.0.0 |
| ---- | ---- |
| `OFFLOAD(...)` | `SOLOMON_OFFLOAD(...)` |
| `END_OFFLOAD` | `SOLOMON_END_OFFLOAD` |
| `OFFLOAD_OUTER_LOOP(...)` | `SOLOMON_OFFLOAD_OUTER_LOOP(...)` |
| `PARALLELIZE_INNER_LOOP(...)` | `SOLOMON_PARALLELIZE_INNER_LOOP(...)` |
| `END_OFFLOAD_OUTER_LOOP` | `SOLOMON_END_OFFLOAD_OUTER_LOOP` |
| `IF_NOT_OFFLOADED(arg)` | `SOLOMON_IF_NOT_OFFLOADED(arg)` |
| `DECLARE_OFFLOADED(...)` | `SOLOMON_DECLARE_OFFLOADED(...)` |
| `DECLARE_OFFLOADED_END` | `SOLOMON_DECLARE_OFFLOADED_END` (not required in Fortran / Fortran では不要) |
| `DECLARE_END_OFFLOADED` | (removed; use `SOLOMON_DECLARE_OFFLOADED_END` / 削除．`SOLOMON_DECLARE_OFFLOADED_END` を使用) |
| `SYNCHRONIZE(...)` | `SOLOMON_SYNCHRONIZE(...)` |
| `WAIT_QUEUE(id)` | `SOLOMON_WAIT_QUEUE(id)` |
| `ATOMIC(...)` | `SOLOMON_ATOMIC(...)` |
| `ATOMIC_UPDATE` | `SOLOMON_ATOMIC_UPDATE` |
| `ATOMIC_READ` | `SOLOMON_ATOMIC_READ` |
| `ATOMIC_WRITE` | `SOLOMON_ATOMIC_WRITE` |
| `ATOMIC_CAPTURE` | `SOLOMON_ATOMIC_CAPTURE` |
| `DATA_ACCESS_BY_HOST(...)` | `SOLOMON_DATA_ACCESS_BY_HOST(...)` |
| `DATA_ACCESS_BY_DEVICE(...)` | `SOLOMON_DATA_ACCESS_BY_DEVICE(...)` |
| `USE_DEVICE_DATA_FROM_HOST(...)` | `SOLOMON_USE_DEVICE_DATA_FROM_HOST(...)` |
| `MALLOC_ON_DEVICE(...)` | `SOLOMON_MALLOC_ON_DEVICE(...)` |
| `ALLOCATE_ON_DEVICE(...)` | `SOLOMON_ALLOCATE_ON_DEVICE(...)` |
| `FREE_FROM_DEVICE(...)` | `SOLOMON_FREE_FROM_DEVICE(...)` |
| `DEALLOCATE_ON_DEVICE(...)` | `SOLOMON_DEALLOCATE_ON_DEVICE(...)` |
| `MEMCPY_D2H(...)` | `SOLOMON_MEMCPY_D2H(...)` |
| `MEMCPY_H2D(...)` | `SOLOMON_MEMCPY_H2D(...)` |
| `DECLARE_DATA_ON_DEVICE(...)` | `SOLOMON_DECLARE_DATA_ON_DEVICE(...)` |

## Clause tokens / 引数トークン

| v1.x | v2.0.0 |
| ---- | ---- |
| `AS_INDEPENDENT` | `SOLOMON_CLAUSE_INDEPENDENT` |
| `AS_SEQUENTIAL` | `SOLOMON_CLAUSE_SEQUENTIAL` |
| `NUM_THREADS(n)` | `SOLOMON_CLAUSE_NUM_THREADS(n)` |
| `NUM_BLOCKS(n)` | `SOLOMON_CLAUSE_NUM_BLOCKS(n)` |
| `NUM_GRIDS(n)` (deprecated) | `SOLOMON_CLAUSE_NUM_GRIDS(n)` (deprecated; use `SOLOMON_CLAUSE_NUM_BLOCKS(n)`) |
| `AS_THREAD` | `SOLOMON_CLAUSE_THREAD` |
| `AS_BLOCK` | `SOLOMON_CLAUSE_BLOCK` |
| `AS_GRID` (deprecated) | `SOLOMON_CLAUSE_GRID` (deprecated; use `SOLOMON_CLAUSE_BLOCK`) |
| `COLLAPSE(n)` | `SOLOMON_CLAUSE_COLLAPSE(n)` |
| `AS_ASYNC(...)` | `SOLOMON_CLAUSE_ASYNC(...)` |
| `ASYNC_QUEUE(id)` | `SOLOMON_CLAUSE_ASYNC_QUEUE(id)` |
| `REDUCTION(...)` | `SOLOMON_CLAUSE_REDUCTION(...)` |
| `ENABLE_IF(condition)` | `SOLOMON_CLAUSE_IF(condition)` |
| `AS_PRIVATE(...)` | `SOLOMON_CLAUSE_PRIVATE(...)` |
| `AS_FIRSTPRIVATE(...)` | `SOLOMON_CLAUSE_FIRSTPRIVATE(...)` |
| `AS_DEVICE_PTR(...)` | `SOLOMON_CLAUSE_DEVICE_PTR(...)` |
| `COPY_BEFORE_AND_AFTER_EXEC(...)` | `SOLOMON_CLAUSE_COPY_BEFORE_AND_AFTER_EXEC(...)` |
| `COPY_H2D_BEFORE_EXEC(...)` | `SOLOMON_CLAUSE_COPY_H2D_BEFORE_EXEC(...)` |
| `COPY_D2H_AFTER_EXEC(...)` | `SOLOMON_CLAUSE_COPY_D2H_AFTER_EXEC(...)` |

## Configuration macros and utilities / 設定マクロ・ユーティリティ

| v1.x | v2.0.0 |
| ---- | ---- |
| `-DOFFLOAD_BY_OPENACC` | `-DSOLOMON_OFFLOAD_BY_OPENACC` (old spelling keeps working / 旧綴りも無条件で有効) |
| `-DOFFLOAD_BY_OPENACC_PARALLEL` | `-DSOLOMON_OFFLOAD_BY_OPENACC_PARALLEL` (same / 同上) |
| `-DOFFLOAD_BY_OPENMP_TARGET` | `-DSOLOMON_OFFLOAD_BY_OPENMP_TARGET` (same / 同上) |
| `-DOFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE` | `-DSOLOMON_OFFLOAD_BY_OPENMP_TARGET_DISTRIBUTE` (same / 同上) |
| `APPEND_ARGS(...)` | `SOLOMON_APPEND_ARGS(...)` |

The two-level notations (`PRAGMA_ACC_*`, `PRAGMA_OMP_*`, `ACC_CLAUSE_*`, `OMP_CLAUSE_*`, `OMP_TARGET_CLAUSE_*`) are unchanged.
2段記法（`PRAGMA_ACC_*`, `PRAGMA_OMP_*`, `ACC_CLAUSE_*`, `OMP_CLAUSE_*`, `OMP_TARGET_CLAUSE_*`）は変更ありません．

## New macros in v2.0.0 / v2.0.0 の新規マクロ

| macro | description |
| ---- | ---- |
| `SOLOMON_CLAUSE_TARGETS(...)` | name the target procedures of `SOLOMON_DECLARE_OFFLOADED(...)` / 手続を名前指定 |
| `SOLOMON_DECLARE_ON_DEVICE(...)` | declare device-resident variables / デバイス常駐変数の宣言 |
| `SOLOMON_DECLARE_ON_DEVICE_LINKED(...)` | device-resident variables with link semantics / link 意味論の常駐変数 |
| `SOLOMON_WITH_SHORT_NAMES` | enable the v1.x spellings / v1.x の綴りを有効化（デフォルト OFF） |
