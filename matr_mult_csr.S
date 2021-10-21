.intel_syntax noprefix
.global matr_mult_csr
.text

// a in RDI, b in RSI, result in RDX
matr_mult_csr:
    push rdi
    push rsi
    push rdx

    mov rdi, [rsi + 8]
    mov rsi, 4
    call calloc
    // rax = Pointer auf currentCol Array

    pop rdx
    pop rsi
    pop rdi

    // Registersicherung
    push r12
    push r13
    push r14
    push r15
    push rbx

    // result.werteUngleich0 = 0, rcx = currentElement
    mov qword ptr [rdx + 16], 0
    xor rcx, rcx

    // r8 = i, r9 = Zeilen von A 
    xor r8, r8
    mov r9, [rdi]
 
    .LrowsOfMatrixA:
        // r10 = allEntriesToZero-Counter
        xor r10, r10
       
        // allEntriesToZero(currentCol)
        .LallEntriesToZero:
            pxor xmm0,xmm0
            movd [rax + r10 * 4], xmm0
            inc r10
            cmp r10, [rsi + 8]
            jl .LallEntriesToZero

        // r10 = A.rowPtr[i] = j
        mov r10, [rdi + 48]
        mov r11, r10
        mov r10, [r10 + r8 * 8]

        // r11 = A.rowPtr[i + 1]
        inc r8
        mov r11, [r11 + r8 * 8]
        dec r8

        .LvaluesInRowMatrixA:
            cmp r11,r10 
            jle .LfinishVal

            // r12 = A.colIndex[j] = bRow
            mov r12, [rdi + 40]
            mov r12, [r12 + r10 * 8]

            // r13 = B.rowPtr[bRow] = k
            mov r13, [rsi + 48]
            mov r14, r13
            mov r13, [r13 + r12 * 8]

            // r14 = B.rowPtr[bRow + 1]
            inc r12
            mov r14, [r14 + r12 * 8]
            dec r12
           
            .Lmultiplikation:
                cmp r14,r13
                jle .LfinishMul

                // r15 = B.values*, xmm0 = B.values[k]
                mov r15, [rsi + 32]
                pxor xmm0, xmm0
                movss xmm0, [r15 + r13 * 4]

                // rbx = A.values*, xmm1 = A.values[j]
                mov rbx, [rdi + 32]
                movss xmm1, [rbx + r10 * 4]

                // xmm0 = B.values[k] * A.values[j]
                mulss xmm0, xmm1

                // r15 = B.colIndex[k]
                mov r15, [rsi + 40]
                mov r15, [r15 + r13 * 8]

                // currentCol[B.colIndex[k]]
                movss xmm1, [rax + r15 * 4]
                addss xmm0, xmm1
                movss [rax + r15 * 4], xmm0

                inc r13
                jmp .Lmultiplikation
           
            .LfinishMul:
            inc r10
            jmp .LvaluesInRowMatrixA
        
        .LfinishVal:
            // r10 = col, xmm0 = 0
            xor r10, r10
            cvtsi2ss xmm2, r10
            xor r10, r10
            push r9

        .Lspeichern:
            // xmm1 = currentCol[col]
            movd xmm1, [rax + r10 * 4]

            ucomiss xmm1, xmm2
            jz .Lfalse
           
            // r11 = result.values
            mov r11, [rdx + 32]
            
            // rcx = result.werteUngleich0 
            push rcx 
            mov rcx, [rdx + 16]

            movd [r11 + rcx * 4], xmm1

            // r11 = result.colIndex
            mov r11, [rdx + 40]
            mov [r11 + rcx * 8], r10
            
            // rcx = currentElement++ 
            inc qword ptr [rdx + 16]
            pop rcx 
            inc rcx

            .Lfalse:
                inc r10

            mov r9,[rsi + 8 ]

            cmp r10, r9
            jl .Lspeichern

        pop r9

        inc r8

        // r11 = result.rowPtr
        mov r11, [rdx + 48]
        mov [r11 + r8 * 8], rcx

        cmp r8, r9
        jl .LrowsOfMatrixA
    
    // free currentCol
    push rdx
    push rdi 
    push rsi
    mov rdi,rax 
    call free 
    pop rsi
    pop rdi
    pop rdx
     
    // result.spalten = B.spalten
    mov rax, [rsi + 8]
    mov [rdx + 8], rax
    
    // result.zeilen = A.zeilen
    mov rax, [rdi]
    mov [rdx], rax
    
    // rcx = result.werteUngleich0 
    mov rcx, [rdx + 16 ]

    // result.values durch realloc wenn nötig kürzen
    push rdx
    lea rsi, [rcx*4]
    mov rdi, [rdx + 32]
    call realloc 
    pop rdx 
    mov [rdx + 32], rax   

    // rcx = result.werteUngleich0 
    mov rcx, [rdx + 16]

    // result.colIndex durch realloc wenn nötig kürzen
    push rdx
    lea rsi, [rcx*8]
    mov rdi, [rdx + 40]
    call realloc 
    pop rdx     
    mov [rdx + 40], rax 
    
    // Registerwiederherstellung
    pop rbx
    pop r15
    pop r14
    pop r13
    pop r12    

    ret