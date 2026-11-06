# Frame dump naming

Recorded frame dumps use this pattern:

`t6_<character>_<move>_<situation>[_<take>].t6dump`

Do not include a date or timestamp.

## Fields

- `character`: lowercase character name.
- `move`: normalized move notation.
- `situation`: `hit`, `ch`, `block`, `whiff`, or `kd`.
- `take`: optional two-digit index for multiple captures, for example `_01`.

## Move normalization

- Use lowercase ASCII only.
- Allowed characters are `a-z`, `0-9`, `_`, and `-`.
- `+` becomes `-`: `1+2` becomes `1-2`.
- `/` is dropped in diagonal prefixes: `d/f+1` becomes `df-1`.
- `,` becomes `_`: `1,2,3` becomes `1_2_3`.
- `~` becomes `-jf-`.
- Spaces, parentheses, quotes, and other symbols become `_`.
- Collapse repeated `_` and `-`, then trim both ends.
- Limit the move token to 64 characters. Append a short hash if truncated or ambiguous.
- Fall back to `move` if the result is empty.

## Examples

- `t6_kazuya_ewgf_ch.t6dump`
- `t6_jin_df-1_block.t6dump`
- `t6_armor-king_1-2_whiff.t6dump`
- `t6_heihachi_f-n-d-df-2_hit.t6dump`
- `t6_jin_df-1_block_01.t6dump`

The filename is a human label, not the data format. Keep exact notation in a manifest or sidecar if it must be preserved.
