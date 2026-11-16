{ Turbo Pascal 3 / CP/M-86 IBM PC timing experiment.
  No runtime patch and no util.lib dependency. See api.md, Delay experiment. }
PROGRAM TDelay;
VAR
  Tick, Started, WaitCount : INTEGER;

PROCEDURE ReadTick;
BEGIN
  INLINE(
    $50/$51/$52/          { PUSH AX,CX,DX }
    $B4/$00/$CD/$1A/      { MOV AH,0; INT 1Ah }
    $89/$16/Tick/         { MOV [Tick],DX }
    $5A/$59/$58);         { POP DX,CX,AX }
END;

PROCEDURE BiosWait(Ticks : INTEGER);
BEGIN
  IF Ticks > 0 THEN BEGIN
    WaitCount := Ticks;
    INLINE(
      $9C/$50/$53/$51/$52/$56/ { PUSHF; PUSH AX,BX,CX,DX,SI }
      $8B/$36/WaitCount/       { MOV SI,[WaitCount] }
      $B4/$00/$CD/$1A/         { MOV AH,0; INT 1Ah }
      $89/$D3/                { MOV BX,DX: start tick }
      $FB/$F4/                { loop: STI; HLT, wake on interrupt }
      $B4/$00/$CD/$1A/         { MOV AH,0; INT 1Ah }
      $29/$DA/                { SUB DX,BX: modulo-65536 elapsed ticks }
      $39/$F2/                { CMP DX,SI }
      $72/$F4/                { JB loop (-12 bytes) }
      $5E/$5A/$59/$5B/$58/$9D); { POP SI,DX,CX,BX,AX; POPF }
  END;
END;

PROCEDURE Report;
BEGIN
  ReadTick;
  WriteLn('BIOS ticks elapsed: ', Tick - Started);
END;

BEGIN
  WriteLn('IBM PC CP/M-86 timing experiment (about 18.2 ticks/second)');
  WriteLn('Run at the normal prompt, with timer interrupts enabled.');
  WriteLn('Turbo Delay(1000):');
  ReadTick; Started := Tick;
  Delay(1000);
  Report;
  WriteLn('Inline BIOS + HLT, 18 ticks:');
  ReadTick; Started := Tick;
  BiosWait(18);
  Report;
  WriteLn('Done. No Turbo runtime or Ladder files changed.');
END.
