.intel_syntax noprefix
.global matr_mult_csr2
.text


matr_mult_csr2:
//Transponierung von B :
    push rbx
    push rdi
    push rdx
    push rsi

//initialising csr B transponated
    mov rdi, 7
    mov rsi, 8

    //ret adress von calloc in rax/rdx
    call calloc
    pop rsi
    //rbx = pointer to bT
    mov rbx,rax
    mov rax, [rsi+8]
    mov [rbx],rax
    mov rax, [rsi]
    mov [rbx+8],rax
    mov rax ,[rsi+16]
    mov [rbx+16],rax

    //initialising the cols rows and vals array for Bt

    push rsi

    mov rdi,[rbx]
    inc rdi
    mov rsi,8
    call calloc
    mov [rbx + 48],rax

    mov rdi,[rbx + 16 ]
    mov rsi,8
    call calloc
    mov [rbx + 40],rax


    mov rdi,[rbx + 16]
    mov rsi,4
    call calloc
    mov [rbx + 32],Rax

    pop rsi
    pop rdx
    pop rdi

// bT initialised

//begin transposition
push r12
push r13
push r14

//r8 = nnz = number of non zero values
xor r8,r8

//r9 = i
xor r9,r9

.LcolsLoop :
    //r10 = brptrT[i] = nnz;
    mov r10,[rbx + 48]
    mov [r10 + r9*8],r8
    //r11 = j
    xor r11,r11
    .LrowsLoop :
    //r12  brptr[j]
    mov r12,[rsi+48]
    mov r13,r12
    mov r12,[r12+r11*8]
    //r13 = brptr[j + 1]
    inc r11
    mov r13,[r13+r11*8]
    dec r11
    .LwhilekInf :
    cmp r13,r12
    jle .LfinishLwhilekInf

        //r14 = bcol[k]
        mov r14,[rsi + 40 ]
        mov r14,[r14+r12*8]
        // if bcol[k] = i
        cmp r14,r9
        jne .LfinishIf
            //rcx = bval[]
            mov rcx,[rsi + 32]
            //xmm0 = bval[k]
            movss xmm0,[rcx + r12*4]
            //rax = bvalT[]
            mov rax,[rbx + 32]
            //bvalT[nnz] = bval[k]
            movss [rax + r8*4 ],xmm0
            //rax = bcolT[]
            mov rax,[rbx + 40]
            //bcolT[nnz] = j;
            mov [rax + r8*8 ],r11
            //nnz++
            inc r8
        .LfinishIf :
        //k++
        inc r12
        jmp .LwhilekInf
    .LfinishLwhilekInf :
    inc r11
    cmp r11,[rbx+8]
    jl .LrowsLoop
//i++
inc r9
cmp r9,[rbx]
jle .LcolsLoop


    pop r14
    pop r13
    pop r12
//B tranponed



//begin multiplication

//rbx= pointer to bT
//rdi poiter to A
//rdx pointer to Result
push r12
push r13
push r14
push r15

//rax = nnz
xor rax,rax
//r9= rptr
xor r9,r9
.LarowsLoop :
   //rcx = result.rowptr
    mov rcx,[rdx +48]
    //result.rowptr[rptr] = nnz
    mov [rcx + r9*8],rax

    //r10 = bptr
    xor r10,r10
    .LbrowsLoop :

    	//xmm2 = val
        pxor xmm2,xmm2

        //r12 = acolptr = arptr[rptr]
        mov r12,[rdi + 48]
        mov r13,r12
        mov r12,[r12 + r9*8]

        //r13 = arptr[rptr + 1]
        inc r9
        mov r13,[r13+r9*8]
        dec r9

        .LacolLoop :
        // acolptr < arptr[rptr + 1]
            cmp r13,r12
            jle .LfinishacolLoop

            //r14 =  bcolptr
            mov r14,[rbx + 48]
            mov r15,r14
            mov r14,[r14 + r10*8]

            //r15 = brptrT[bptr+1]
            inc r10
            mov r15,[r15+r10*8]
            dec r10

            .Lwhile:
            //bcolptr < brptrT[bptr+1]
                cmp r15,r14
                jle .LfinishWhile
                    //rbx = bcolT[bcolptr]
                    mov r8,[rbx + 40]
                    mov r8,[r8 + r14*8]

                    //rcx =acol[acolptr]
                    mov rcx,[rdi + 40 ]
                    mov rcx,[rcx + r12*8]
                    //if acol[acolptr] = bcolT[bcolptr]
                    cmp r8,rcx
                    jne .Lfalse
                    //aval[acolptr]
                    mov rcx,[rdi + 32 ]
                    movss xmm0,[rcx+r12*4]
                    //bvalT[bcolptr]
                    mov r8,[rbx + 32]
                    movss xmm1,[r8 + r14*4]
                    //val += bvalT[bcolptr]*aval[acolptr]
                    mulss xmm0,xmm1
                    addss xmm2,xmm0
                    jmp  .LfinishWhile
                    .Lfalse :
                    //bcolptr++
                    inc r14
                    jmp .Lwhile
            .LfinishWhile :
            //acolptr++
            inc r12
            jmp .LacolLoop
            //make the addition and mult
        .LfinishacolLoop :
        //if(val!=0)
        pxor xmm0,xmm0
        ucomiss xmm2,xmm0
        je .Lvalnull
        //cval[nnz]= val
        mov r11,[rdx + 32 ]
        movss [r11 + rax*4],  xmm2

        // col[nnz] =bptr
        mov r11,[rdx + 40 ]
        mov [r11 + rax*8],r10

        //nnz++
        inc rax
        .Lvalnull :
        //bptr++
        inc r10
        cmp r10,[rbx]
        jl .LbrowsLoop
    //rptr++
    inc r9
    cmp r9,[rdi]
    jl .LarowsLoop

    //result.rowptr[arows]=nnz;
    mov r8,[rdx + 48]
    mov rcx,[rdi]
    mov [r8 + rcx*8],rax

    //result.nnz = nnz
    mov [rdx + 16 ],rax

    //result.cols = number of cols of B
     mov rax,[rsi + 8 ]
     mov [rdx + 8 ],rax

    //result.rows = number of rows of A
     mov rax,[rdi]
     mov [rdx],rax

    // rcx = result.werteUngleich0
    mov rcx, [rdx + 16 ]

    // result.values with realloc shrink if possible 
    push rdi
    push rsi
    push rdx
    lea rsi, [rcx*4]
    mov rdi, [rdx + 32]
    call realloc
    pop rdx
    mov [rdx + 32], rax
    // rcx = nnz 
    mov rcx, [rdx + 16]

    push rdx
    // result.values with realloc shrink if possible 
    lea rsi, [rcx*8]
    mov rdi, [rdx + 40]
    call realloc
    pop rdx
    mov [rdx + 40], rax



  //Free transpned B
    push rdx
    mov rdi,[rbx+48]
    call free
    mov rdi,[rbx+40]
    call free
    mov rdi,[rbx+32]
    call free
    mov rdi,rbx
    call free


    pop rdx
    pop rsi
    pop rdi

    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    ret