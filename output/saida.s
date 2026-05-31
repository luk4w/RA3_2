    .syntax unified

    .data
    STACK_RES: .space 8000      @ Pilha para guardar os 1000 ultimos resultados
    STACK_RES_TOP:              @ Topo da pilha
    LIT_ONE_POW: .double 1.0    @ Constante 1 para o acumulador da potenciacao
    LIT_12: .double 0
    LIT_13: .double 1
    LIT_8: .double 10
    LIT_3: .double 10.0
    LIT_0: .double 10.5
    LIT_11: .double 100.0
    LIT_6: .double 15
    LIT_10: .double 2
    LIT_15: .double 2.0
    LIT_1: .double 2.5
    LIT_9: .double 3
    LIT_4: .double 3.0
    LIT_7: .double 4
    LIT_5: .double 4.0
    LIT_14: .double 5
    LIT_2: .double 50.0
    VAR_CONTADOR: .double 0.0
    VAR_GLOBAL: .double 0.0

    .text
    .global _start
_start:

    @ Inicializa o Stack Pointer alinhado em 8 bytes para VFP
    LDR SP, =0x3FFFFFF8

    @ Inicializa o RESult stackpointer
    LDR R10, =STACK_RES_TOP

    @ Percorre a AST em pos-ordem (equivale a avaliacao RPN)
    @ Empilha Constante: 10.5
    LDR R0, =LIT_0
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Empilha Constante: 2.5
    LDR R0, =LIT_1
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Soma
    VPOP.F64 {D1}           @ POP operando B
    VPOP.F64 {D0}           @ POP operando A
    VADD.F64 D2, D0, D1     @ A + B
    VPUSH.F64 {D2}          @ PUSH pilha

    @ salva o resultado no historico de memoria
    VPOP.F64 {D0}           @ POP resultado final da FPU
    SUB R10, R10, #8        @ Avanca o ponteiro da pilha de historico
    VSTR.F64 D0, [R10]      @ Grava na RAM

    @ Empilha Constante: 50.0
    LDR R0, =LIT_2
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Empilha Constante: 10.0
    LDR R0, =LIT_3
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Subtracao
    VPOP.F64 {D1}           @ POP operando B
    VPOP.F64 {D0}           @ POP operando A
    VSUB.F64 D2, D0, D1     @ A - B
    VPUSH.F64 {D2}          @ PUSH pilha

    @ salva o resultado no historico de memoria
    VPOP.F64 {D0}           @ POP resultado final da FPU
    SUB R10, R10, #8        @ Avanca o ponteiro da pilha de historico
    VSTR.F64 D0, [R10]      @ Grava na RAM

    @ Empilha Constante: 3.0
    LDR R0, =LIT_4
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Empilha Constante: 4.0
    LDR R0, =LIT_5
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Multiplicacao
    VPOP.F64 {D1}           @ POP B
    VPOP.F64 {D0}           @ POP A
    VMUL.F64 D2, D0, D1     @ D2 = A * B
    VPUSH.F64 {D2}          @ PUSH

    @ salva o resultado no historico de memoria
    VPOP.F64 {D0}           @ POP resultado final da FPU
    SUB R10, R10, #8        @ Avanca o ponteiro da pilha de historico
    VSTR.F64 D0, [R10]      @ Grava na RAM

    @ Empilha Constante: 10.0
    LDR R0, =LIT_3
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Empilha Constante: 3.0
    LDR R0, =LIT_4
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Divisao Real (|)
    VPOP.F64 {D1}           @ POP B
    VPOP.F64 {D0}           @ POP A
    VDIV.F64 D2, D0, D1     @ D2 = A / B
    VPUSH.F64 {D2}          @ PUSH

    @ salva o resultado no historico de memoria
    VPOP.F64 {D0}           @ POP resultado final da FPU
    SUB R10, R10, #8        @ Avanca o ponteiro da pilha de historico
    VSTR.F64 D0, [R10]      @ Grava na RAM

    @ Empilha Constante: 15
    LDR R0, =LIT_6
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Empilha Constante: 4
    LDR R0, =LIT_7
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Divisao Inteira (/)
    VPOP.F64 {D1}           @ POP B
    VPOP.F64 {D0}           @ POP A
    VDIV.F64 D2, D0, D1     @ D2 = A / B
    VCVT.S32.F64 S4, D2     @ Trunca o resultado para inteiro
    VCVT.F64.S32 D2, S4     @ Converte de volta para Double (sem decimais)
    VPUSH.F64 {D2}          @ PUSH

    @ salva o resultado no historico de memoria
    VPOP.F64 {D0}           @ POP resultado final da FPU
    SUB R10, R10, #8        @ Avanca o ponteiro da pilha de historico
    VSTR.F64 D0, [R10]      @ Grava na RAM

    @ Empilha Constante: 10
    LDR R0, =LIT_8
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Empilha Constante: 3
    LDR R0, =LIT_9
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Modulo (%)
    VPOP.F64 {D1}           @ POP B
    VPOP.F64 {D0}           @ POP A
    VDIV.F64 D2, D0, D1     @ D2 = A / B
    VCVT.S32.F64 S4, D2     @ S4 = parte inteira de (A/B)
    VCVT.F64.S32 D3, S4     @ D3 = parte inteira em Double
    VMUL.F64 D4, D1, D3     @ D4 = B * parte_inteira(A/B)
    VSUB.F64 D5, D0, D4     @ D5 = A - D4 (sobra o resto)
    VPUSH.F64 {D5}          @ PUSH Resto

    @ salva o resultado no historico de memoria
    VPOP.F64 {D0}           @ POP resultado final da FPU
    SUB R10, R10, #8        @ Avanca o ponteiro da pilha de historico
    VSTR.F64 D0, [R10]      @ Grava na RAM

    @ Empilha Constante: 2
    LDR R0, =LIT_10
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Empilha Constante: 3
    LDR R0, =LIT_9
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Potenciacao (^)
    VPOP.F64 {D1}           @ POP B (Expoente)
    VPOP.F64 {D0}           @ POP A (Base)
    VCVT.S32.F64 S2, D1     @ Converte expoente para inteiro (S2)
    VMOV R4, S2             @ Move expoente para R4 (Contador de Hardware)
    LDR R5, =LIT_ONE_POW    @ Carrega ponteiro do 1.0
    VLDR.F64 D2, [R5]       @ D2 = 1.0 (Acumulador neutro)
pow_loop_0:
    CMP R4, #0              @ Verifica se o contador e <= 0
    BLE pow_end_0   @ Quebra o laco se o expoente esgotar
    VMUL.F64 D2, D2, D0     @ Acumula: D2 = D2 * Base
    SUB R4, R4, #1          @ Decrementa contador
    B pow_loop_0    @ Retorna pro laco
pow_end_0:
    VPUSH.F64 {D2}          @ PUSH resultado

    @ salva o resultado no historico de memoria
    VPOP.F64 {D0}           @ POP resultado final da FPU
    SUB R10, R10, #8        @ Avanca o ponteiro da pilha de historico
    VSTR.F64 D0, [R10]      @ Grava na RAM

    @ Empilha Constante: 100.0
    LDR R0, =LIT_11
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Grava resultado na variavel GLOBAL
    VPOP.F64 {D0}
    LDR R0, =VAR_GLOBAL
    VSTR.F64 D0, [R0]

    @ LOAD: Le a variavel GLOBAL da memoria e empilha
    LDR R0, =VAR_GLOBAL
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ salva o resultado no historico de memoria
    VPOP.F64 {D0}           @ POP resultado final da FPU
    SUB R10, R10, #8        @ Avanca o ponteiro da pilha de historico
    VSTR.F64 D0, [R10]      @ Grava na RAM

    @ Empilha Constante: 0
    LDR R0, =LIT_12
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Operador RES: Busca historico na memoria
    VPOP.F64 {D0}           @ Pega o N da pilha
    VCVT.S32.F64 S0, D0     @ Converte N de Double para Inteiro
    VMOV R1, S0             @ MOV N pro R1
    LSL R1, R1, #3          @ Multiplica N por 8 bytes (descobrir o offset)
    ADD R2, R10, R1         @ Soma o offset com o R10 para achar o endereco antigo
    VLDR.F64 D1, [R2]       @ Le o historico da RAM
    VPUSH.F64 {D1}          @ PUSH pilha

    @ salva o resultado no historico de memoria
    VPOP.F64 {D0}           @ POP resultado final da FPU
    SUB R10, R10, #8        @ Avanca o ponteiro da pilha de historico
    VSTR.F64 D0, [R10]      @ Grava na RAM

    @ Empilha Constante: 10
    LDR R0, =LIT_8
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Empilha Constante: 10
    LDR R0, =LIT_8
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Comparacao FPU
    VPOP.F64 {D1}           @ Operando B
    VPOP.F64 {D0}           @ Operando A
    VCMP.F64 D0, D1         @ Compara A com B
    VMRS APSR_nzcv, FPSCR   @ Transfere Flags da FPU para o processador

    BNE else_label_1
    @ Empilha Constante: 1
    LDR R0, =LIT_13
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Operador RES: Busca historico na memoria
    VPOP.F64 {D0}           @ Pega o N da pilha
    VCVT.S32.F64 S0, D0     @ Converte N de Double para Inteiro
    VMOV R1, S0             @ MOV N pro R1
    LSL R1, R1, #3          @ Multiplica N por 8 bytes (descobrir o offset)
    ADD R2, R10, R1         @ Soma o offset com o R10 para achar o endereco antigo
    VLDR.F64 D1, [R2]       @ Le o historico da RAM
    VPUSH.F64 {D1}          @ PUSH pilha

    B end_if_1
else_label_1:
    @ Empilha Constante: 0
    LDR R0, =LIT_12
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Operador RES: Busca historico na memoria
    VPOP.F64 {D0}           @ Pega o N da pilha
    VCVT.S32.F64 S0, D0     @ Converte N de Double para Inteiro
    VMOV R1, S0             @ MOV N pro R1
    LSL R1, R1, #3          @ Multiplica N por 8 bytes (descobrir o offset)
    ADD R2, R10, R1         @ Soma o offset com o R10 para achar o endereco antigo
    VLDR.F64 D1, [R2]       @ Le o historico da RAM
    VPUSH.F64 {D1}          @ PUSH pilha

end_if_1:

    @ Empilha Constante: 0
    LDR R0, =LIT_12
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Grava resultado na variavel CONTADOR
    VPOP.F64 {D0}
    LDR R0, =VAR_CONTADOR
    VSTR.F64 D0, [R0]

while_start_2:
    @ LOAD: Le a variavel CONTADOR da memoria e empilha
    LDR R0, =VAR_CONTADOR
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Empilha Constante: 5
    LDR R0, =LIT_14
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Comparacao FPU
    VPOP.F64 {D1}           @ Operando B
    VPOP.F64 {D0}           @ Operando A
    VCMP.F64 D0, D1         @ Compara A com B
    VMRS APSR_nzcv, FPSCR   @ Transfere Flags da FPU para o processador

    BGT while_end_2
    @ LOAD: Le a variavel CONTADOR da memoria e empilha
    LDR R0, =VAR_CONTADOR
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Empilha Constante: 1
    LDR R0, =LIT_13
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Soma
    VPOP.F64 {D1}           @ POP operando B
    VPOP.F64 {D0}           @ POP operando A
    VADD.F64 D2, D0, D1     @ A + B
    VPUSH.F64 {D2}          @ PUSH pilha

    @ Grava resultado na variavel CONTADOR
    VPOP.F64 {D0}
    LDR R0, =VAR_CONTADOR
    VSTR.F64 D0, [R0]

    B while_start_2
while_end_2:

    @ LOAD: Le a variavel CONTADOR da memoria e empilha
    LDR R0, =VAR_CONTADOR
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ salva o resultado no historico de memoria
    VPOP.F64 {D0}           @ POP resultado final da FPU
    SUB R10, R10, #8        @ Avanca o ponteiro da pilha de historico
    VSTR.F64 D0, [R10]      @ Grava na RAM

    @ Empilha Constante: 10.0
    LDR R0, =LIT_3
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Empilha Constante: 10.0
    LDR R0, =LIT_3
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Multiplicacao
    VPOP.F64 {D1}           @ POP B
    VPOP.F64 {D0}           @ POP A
    VMUL.F64 D2, D0, D1     @ D2 = A * B
    VPUSH.F64 {D2}          @ PUSH

    @ Empilha Constante: 2.0
    LDR R0, =LIT_15
    VLDR.F64 D0, [R0]
    VPUSH.F64 {D0}

    @ Divisao Real (|)
    VPOP.F64 {D1}           @ POP B
    VPOP.F64 {D0}           @ POP A
    VDIV.F64 D2, D0, D1     @ D2 = A / B
    VPUSH.F64 {D2}          @ PUSH

    @ salva o resultado no historico de memoria
    VPOP.F64 {D0}           @ POP resultado final da FPU
    SUB R10, R10, #8        @ Avanca o ponteiro da pilha de historico
    VSTR.F64 D0, [R10]      @ Grava na RAM

    @ Trava o ultimo resultado do historico no D15 para os LEDs
    VLDR.F64 D15, [R10]

_interactive_loop:
    LDR R0, =0xFF200050     @ Endereco dos Botoes
    LDR R1, [R0]            @ Le botoes
    VMOV R2, R3, D15        @ Extrai bits brutos R3=High, R2=Low
    MOV R4, #0              @ Default: Apaga todos os LEDs
    TST R1, #1              @ KEY0 pressionado (Bit 0)?
    MOVNE R4, R2            @ Sim, sobrepoe R4 com a Word Baixa
    TST R1, #2              @ KEY1 pressionado (Bit 1)?
    MOVNE R4, R3            @ Sim, sobrepoe R4 com a Word Alta
    LDR R5, =0xFF200000     @ Endereco dos LEDs
    STR R4, [R5]            @ Atualiza os LEDs
    B _interactive_loop     @ Watchdog do loop
