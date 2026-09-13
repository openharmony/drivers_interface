# AGENTS.md — drivers/interface/partitionslot

Guidance for coding agents working in the OpenHarmony **drivers_interface_partitionslot**
component — the HDI interface definition (IDL) of the PartitionSlot service
(A/B multi-slot boot partition management). Read this before editing any file
under `drivers/interface/partitionslot/`.

> Scope: component root (`drivers/interface/partitionslot`), inside the
> `drivers/interface` git repository. The server-side implementation lives in
> a separate repo: `drivers/peripheral/partitionslot` (which has its own
> AGENTS.md). The Chinese translation `AGENTS_zh.md` is content-equivalent;
> if the two diverge, the Chinese version prevails.

## 1. Code structure

### 1.1 What this component owns

The PartitionSlot HDI interface definition — four APIs, @since 3.1:
- `GetCurrentSlot` — which partition slot the board booted from, and how many slots exist,
- `GetSlotSuffix` — slot index → partition-name suffix,
- `SetActiveSlot` — switch the slot the device boots from next,
- `SetSlotUnbootable` — mark a slot unbootable.

Interface definitions only — three files: `bundle.json`, `v1_0/BUILD.gn`,
`v1_0/IPartitionSlot.idl`. No implementation code here. The `hdi()` build
template compiles the IDL into proxy/stub C++ and exports three `inner_kits`
targets that downstream components link: `libpartitionslot_proxy_1.0` (client
side), `libpartitionslot_stub_1.0` (server side), `partitionslot_idl_headers`.
Typical consumers: the service implementation in
`drivers/peripheral/partitionslot`, the on-device test `partitionslot_hdi_test`,
and the `begetctl partitionslot` tool in `base/startup/init`.

### 1.2 Build-time generation chain (dependency direction: top generates bottom; never hand-edit the bottom)

```
v1_0/IPartitionSlot.idl            package ohos.hdi.partitionslot.v1_0 (the only API file)
  → hdi() template                 //build/config/components/hdi/hdi.gni (codegen + versioning rules)
  → out/<product>/gen/drivers/interface/partitionslot/v1_0/   generated proxy/stub C++ (read-only)
  → libpartitionslot_proxy_1.0 / libpartitionslot_stub_1.0 / partitionslot_idl_headers   (inner_kits)
  → consumers                      drivers/peripheral/partitionslot (service impl),
                                   partitionslot_hdi_test, begetctl partitionslot (startup_init)
```

| Path | Product | Role |
| --- | --- | --- |
| `v1_0/IPartitionSlot.idl` | — (source of everything) | The only API file; per-method doc comments define slot semantics and the return-value contract |
| `v1_0/BUILD.gn` | `partitionslot_idl_target` (+ generated libs) | `hdi()` template wiring; where `install_images` is declared |
| `bundle.json` | — | `sub_component` + `inner_kits` registration — the downstream-facing surface |
| `out/<product>/gen/drivers/interface/partitionslot/v1_0/` | generated C++ | Ground truth for exact C++ signatures and IPC layout — read-only, regenerate by building |
| *(cross-repo)* `drivers/peripheral/partitionslot` | service impl | Implements this interface; its AGENTS.md documents bootctrl layout and the slot-numbering quirk |
| *(cross-repo)* `drivers/peripheral/partitionslot/test/unittest/hdf_partitionslot_test.cpp` | `partitionslot_hdi_test` | Consumer-side test; links `libpartitionslot_proxy_1.0` |

High-risk / frequently-changed paths (treat changes here with extra care):
`v1_0/IPartitionSlot.idl` (the public HDI API), `bundle.json` (`inner_kits`
is the downstream-facing surface).

Nested guidance: none in this repo. The implementation side has its own
`drivers/peripheral/partitionslot/AGENTS.md`.

## 2. Knowledge routing — read before you edit

Before editing any file, state to yourself: (1) the task category, (2) which row
of §2.1 you loaded (plus the IDL row whenever API semantics are involved), and
(3) which §3 constraint applies. If you cannot answer all three, stop and read.

### 2.1 Task-based routing

| Working on / task | Read this first | Key concept to load |
| --- | --- | --- |
| Add or extend an HDI API | `v1_0/IPartitionSlot.idl` + `//build/config/components/hdi/hdi.gni` + `drivers/interface/connected_nfc_tag/v1_1/` | `extends` syntax; v1_1 minor-version flow (never append to released v1_0) |
| Build wiring, code generation | `v1_0/BUILD.gn` + the `hdi()` template in `hdi.gni` | What the template generates; `install_images` is publish-facing |
| Component registration, exported libs | `bundle.json` + `drivers/interface/connected_nfc_tag/bundle.json` | `sub_component` + `inner_kits` multi-version registration |
| Verify generated C++ signatures / IPC layout | `out/<product>/gen/drivers/interface/partitionslot/v1_0/` | Generated code is the ground truth — don't guess the layout |
| Implementation / HAL behavior | `drivers/peripheral/partitionslot` + its AGENTS.md | bootctrl disk layout; slot numbering treats 2 as slot B |
| Test cases / expectations | `drivers/peripheral/partitionslot/test/unittest/hdf_partitionslot_test.cpp` | One case per API; requires a real device (HDF + bootctrl) |
| Downstream consumer impact | `bundle.json` `inner_kits` + grep `drivers_interface_partitionslot` across the tree | Who links the proxy/stub libraries |

### 2.2 Vocabulary routing (when a term appears in a task/log/issue/API)

| Term / acronym | Concept |
| --- | --- |
| slot, A/B partition, active slot, unbootable, slot suffix | Defined per method in `IPartitionSlot.idl`; return-value contract in §3.1 |
| HDI, proxy, stub, `inner_kits`, `hdi()` | HDF IPC machinery; semantics defined by the `hdi()` template in `hdi.gni` and the generated code under `out/<product>/gen/` |
| v1_0 / v1_1 / major-minor versioning | Versioning rules in `hdi.gni`; `connected_nfc_tag/v1_1` is the worked extension example |
| bootctrl, `bootslots`, slot numbering | Implementation-side terms — see `drivers/peripheral/partitionslot/AGENTS.md` |

## 3. Constraints and boundaries (do not break without escalation)

### 3.1 Architecture invariants (hard rules)

1. **The package declaration must match the directory path.**
   `ohos.hdi.partitionslot.v1_0` ↔ `drivers/interface/partitionslot/v1_0/` —
   a mismatch breaks code generation or generates into the wrong path.
2. **Released v1_0 is frozen.** Never change the signature, parameter
   direction labels, or return-value semantics of existing methods — source-
   and IPC-level compatibility must hold within the major version. Extensions
   go to a new `v1_1/` minor version via `extends` (see §3.4).
3. **Return-value contract.** `0` = success, negative = failure; `-1`
   specifically means the board does not support A/B partitions. For
   `GetCurrentSlot` the return value *is* the current slot number (`-1` no A/B,
   `0` slot A, `1` slot B, `N` slot N).
4. **Every parameter carries a direction label**: `[in]`, `[out]` or `[in, out]`.
5. **Naming**: methods UpperCamelCase, parameters lowerCamelCase,
   constants/enums ALL_CAPS.

### 3.2 Do-not rules

- Do **not** edit generated code under `out/<product>/gen/` — regenerate by
  building instead; a hand "fix" is lost on the next build.
- Do **not** modify existing `v1_0` method signatures, direction labels, or
  return-value semantics (§3.1.2).
- Do **not** add methods to the released `v1_0` — create a `v1_1/` minor
  version instead.
- Do **not** change the return-value meanings, including the `-1` special case
  (§3.1.3).
- Do **not** introduce third-party dependencies not declared in `bundle.json`.
- Do **not** remove the Apache 2.0 license header; new files must carry it
  (see the header of `IPartitionSlot.idl`).

### 3.3 Ask before

- Any change to `v1_0` files beyond comment/typo fixes.
- Any change to `bundle.json` `sub_component` or `inner_kits` — that is what
  downstream components link.
- Introducing a new major version (`v2_0/`) or removing/deprecating an API.
- Changing `install_images` in `v1_0/BUILD.gn` (affects which partition images
  ship the libraries).
- Any semantic change to `SetActiveSlot` / `SetSlotUnbootable` — they change
  which partition the device boots from; needs maintainer sign-off.

### 3.4 Local conventions

- Minor-version extension syntax:
  `interface IPartitionSlot extends ohos.hdi.partitionslot.v1_0.IPartitionSlot { ... }`,
  then register the new targets in `bundle.json` (`sub_component` + `inner_kits`),
  following `drivers/interface/connected_nfc_tag/v1_1` as the reference. A new
  `v1_1/` directory without `bundle.json` registration silently builds nothing.
- IDL doc comments follow the existing style in `IPartitionSlot.idl`:
  `@brief` / `@param` / `@return` / `@since` blocks above each method.
- This guidance is a bilingual pair: `AGENTS.md` (English) and `AGENTS_zh.md`
  (Chinese), content-equivalent; keep both in sync when editing.

### 3.5 Common agent failure modes here

- Running `build.sh` from this directory — it only works from the OHOS source
  root (ancestor containing `build.sh`).
- Creating `v1_1/` without updating `bundle.json` — a silently dead target.
- "Fixing" a signature by editing generated files under `out/` — lost on the
  next build.
- Expecting this repo to describe runtime behavior — implementation semantics
  (bootctrl layout, slot numbering quirk) live in
  `drivers/peripheral/partitionslot/AGENTS.md`.

## 4. Verification loop

### 4.1 Build the component (from the OHOS source root, not this directory)

```bash
./build.sh --product-name rk3568 --build-target partitionslot_idl_target   # IDL + generated proxy/stub
```

### 4.2 Build the downstream consumer test

```bash
./build.sh --product-name rk3568 --build-target partitionslot_hdi_test
# binary: out/rk3568/tests/unittest/drivers_peripheral_partitionslot/drivers_peripheral_partition_slot/partitionslot_hdi_test
```

The test links `libpartitionslot_proxy_1.0` — a passing build proves the
interface still compiles and links end to end. Running the binary needs a real
device (HDF environment + bootctrl); without a device, build-only checks are
the honest limit — do not present a passing build as runtime verification.

### 4.3 Minimum checks before declaring done

- [ ] `--build-target partitionslot_idl_target` build succeeds.
- [ ] Interface change: `--build-target partitionslot_hdi_test` also builds.
- [ ] `git diff` shows no released `v1_0` signature change (comment/typo fixes
      only) and no `bundle.json` `inner_kits` / `install_images` change unless
      explicitly approved (§3.3).
- [ ] After any IDL change: read the regenerated code under
      `out/<product>/gen/drivers/interface/partitionslot/v1_0/` and confirm
      the diff contains only intended changes.
- [ ] New files carry the Apache 2.0 header; a new `v1_1/` is registered in
      `bundle.json`.

### 4.4 Done definition & final response

Report: (1) files changed with `path:line` of the key edits, (2) which §3
constraint applied and how it was respected, (3) the exact build/test commands
run and their pass/fail result. If the build/tests could not run (no toolchain
or device), say so explicitly, mark the work `NOT VERIFIED`, and list the
commands a human should run instead — do not claim success without evidence.
