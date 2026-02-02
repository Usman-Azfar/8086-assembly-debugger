# Register Command Emulator

A powerful 8086 Assembly project designed to emulate register value inspection and modification. This tool provides a command-line interface to interact with internal CPU registers and flags, similar to the `debug` command in MS-DOS.

## 🚀 Features

- **Register Inspection**: View values for `AX`, `BX`, `CX`, `DX`, `SP`, `BP`, `SI`, `DI`, `DS`, `ES`, `SS`, `CS`, and `IP`.
- **Flag Monitoring**: Displays active CPU flags (OV, UP, EI, NG, ZR, AC, PO, CY).
- **Register Modification**: Directly modify register values using hexadecimal input.
- **Next Instruction Preview**: Displays the upcoming instruction in memory.
- **Case Insensitivity**: Commands and registers are recognized regardless of case.

## 🛠️ How to Run

1.  **Environment**: You will need an environment that supports 16-bit DOS applications (e.g., **DOSBox**).
2.  **Assembler**: Use an assembler like **MASM** (Macro Assembler) or **TASM** (Turbo Assembler).
3.  **Steps**:
    - Assemble the source: `masm CProj.asm;`
    - Link the object file: `link CProj.obj;`
    - Run the executable: `CProj.exe`

## ⌨️ Command Usage

Once started, the program waits for a command:

- `r`: Displays the current status of all registers and flags.
- `r [register_name]`: Prompts to modify the value of a specific register (e.g., `r ax`).
- `q`: Exits the program.

### Example:
```text
Press r/R (register command) to start or q/Q (exit command) to exit : r

AX=0000  BX=0000  CX=0000  DX=0000  SP=0000  BP=0000  SI=0000  DI=0000
DS=0000  ES=0000  SS=0000  CS=0000  IP=0000  NV UP EI PL NZ NA PO NC
-r ax
AX 0000
:1234
```

## 📜 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
