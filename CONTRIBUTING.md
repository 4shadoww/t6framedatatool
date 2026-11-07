# Contributing

## Supported game version

Only Tekken 6 Europe (BLES00635) with the 01.03 update patch is supported.

Bug reports for other regions, discs, PSN versions, or patch levels will not
be fixed.

## Reporting bugs

Include:

- What you expected to happen
- What actually happened
- Steps to reproduce
- Character, move, and game state
- Emulator and game version
- Relevant logs

## Frame data inconsistencies

Frame-data bugs require full frame dump and verbose logs.
A report missing any of these items may be closed:

1. **Verbose logs and frame dump**

   Record the affected sequence with both verbose logging and frame dumping:

   ```sh
   ./t6framedata --verbose --dump-frames path/to/capture.t6dump 2> analyser.log
   ```

   Windows PowerShell:

   ```powershell
   .\t6framedata.exe --verbose --dump-frames path\to\capture.t6dump 2> analyser.log
   ```

   Windows cmd:

   ```bat
   t6framedata.exe --verbose --dump-frames path\to\capture.t6dump 2> analyser.log
   ```

   Attach both `analyser.log` and the `.t6dump` file.

2. **Move string**

   Provide the exact input, including the stance prefix when applicable.

   Examples: `DES.1+2`, `f,n,d,df+4`, `db+2`.

3. **Description**

   Explain:

   - expected startup, active frames, advantage, or knockdown result
   - actual values shown by the analyser
   - whether the move hit, was blocked, or counter-hit
   - attacker side (left or right)
   - any relevant training-mode settings

## Frame dump naming

Use the convention documented in `test/test_data/README.md`.

## Commit messages

Commits must follow the Conventional Commits format:

```text
<type>(<scope>): <description>
```

Common types:

- `feat`: new feature
- `fix`: bug fix
- `docs`: documentation
- `test`: tests
- `refactor`: code change without behaviour change
- `chore`: build or tooling change

Examples:

- `fix(analyser): correct knockdown detection`
- `test(frame-data): add Yoshimitsu DGF.2 case`
- `docs(contributing): add bug reporting guide`

## Submitting fixes

- Keep changes focused.
- Include or update tests when behaviour changes.
- Run:

  ```sh
  make rat
  ```
