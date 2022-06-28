#ifndef REDEFINE_VALUES
    #define VAL     val
    #define VAL_1   val1
    #define VAL_2   val2
    #define OFFSET  displacement
    #define IP      CPU->ip

    // #define SFML    mainData
    // #define WINDOW  SFML->window
    // #define CIRCLE  SFML->circle
#endif

// #define GOUT(indexToOut)            cpuGOut(CPU, indexToOut)

#define PUSH(value)                 cpuPush(CPU, (double) value)
#define POP()                       cpuPop(CPU)
#define OUT()                       cpuOut(CPU)
#define OUTC()                      cpuOutc(CPU)
#define IN()                        cpuIn(CPU)
#define EXIT(exitCode)              cpuExit(CPU, byteCode, exitCode)

#define GET_VALUE()                 cpuGetBytecodeValue(CPU, byteCode)
#define GET_OFFSET()                cpuGetBytecodeOffset(CPU, byteCode)
#define MOVE_VALUE(value)           cpuMoveValue(CPU, byteCode, value)
#define READ_STACK_VALUE(saveTo)    saveTo = POP(); PUSH(saveTo);

// opMnemonics, opcode, argc, code

OPDEF(push, 0, 1, {
    VAL = GET_VALUE();
    PUSH(VAL);
})

OPDEF(pop, 1, 1, {
    VAL = POP();
    MOVE_VALUE(VAL);
})

OPDEF(add, 2, 0, {
    VAL_1 = POP();
    VAL_2 = POP();
    PUSH(VAL_1 + VAL_2);
})

OPDEF(sub, 3, 0, {
    VAL_1 = POP();
    VAL_2 = POP();
    PUSH(VAL_1 - VAL_2);
})

OPDEF(mul, 4, 0, {
    VAL_1 = POP();
    VAL_2 = POP();
    PUSH(VAL_1 * VAL_2);
})

OPDEF(div, 5, 0, {
    VAL_1 = POP();
    VAL_2 = POP();
    PUSH(VAL_1 / VAL_2);
})

OPDEF(out, 6, 0, {
    OUT();
})

OPDEF(in, 7, 0, {
    IN();
})

OPDEF(jmp, 8, 1, {
    OFFSET = GET_OFFSET();
    IP = OFFSET;
})

OPDEF(call, 9, 1, {
    PUSH(IP + sizeof(OFFSET));
    OFFSET = GET_OFFSET();
    IP = OFFSET;
})

OPDEF(ret, 10, 0, {
    OFFSET = (offset) POP();
    IP = OFFSET;
})

OPDEF(outc, 11, 0, {
    OUTC();
})

OPDEF(sqrt, 12, 0, {
    VAL = sqrt(POP());
    PUSH(VAL);
})

OPDEF(je, 13, 1, {
    READ_STACK_VALUE(VAL);
    if (fabs(VAL - DOUBLES_ARE_EQUAL) < EPS)
    {
        POP();

        OFFSET = GET_OFFSET();
        IP = OFFSET;
    }
    else
    {
        IP += sizeof(OFFSET);
    }
})

OPDEF(jl, 14, 1, {
    READ_STACK_VALUE(VAL);
    if (fabs(VAL - FIRST_DOUBLE_IS_LOWER) < EPS)
    {
        POP();

        OFFSET = GET_OFFSET();
        IP = OFFSET;
    }
    else
    {
        IP += sizeof(OFFSET);
    }
})

OPDEF(cmp, 15, 0, {
    VAL_1 = POP();
    VAL_2 = POP();
    PUSH(VAL_2);
    PUSH(VAL_1);

    if (fabs(VAL_1 - VAL_2) < EPS)
    {
        PUSH(DOUBLES_ARE_EQUAL);
    }
    else if (VAL_1 > VAL_2)
    {
        PUSH(FIRST_DOUBLE_IS_GREATER);
    }
    else
    {
        PUSH(FIRST_DOUBLE_IS_LOWER);
    }
})

OPDEF(jg, 16, 1, {
    READ_STACK_VALUE(VAL);
    if (fabs(VAL - FIRST_DOUBLE_IS_GREATER) < EPS)
    {
        POP();

        OFFSET = GET_OFFSET();
        IP = OFFSET;
    }
    else
    {
        IP += sizeof(OFFSET);
    }
})

OPDEF(jne, 17, 1, {
    READ_STACK_VALUE(VAL);
    if (fabs(VAL - DOUBLES_ARE_EQUAL) > EPS)
    {
        POP();

        OFFSET = GET_OFFSET();
        IP = OFFSET;
    }
    else
    {
        IP += sizeof(OFFSET);
    }
})

OPDEF(halt, 255, 0, {
    EXIT(EXIT_SUCCESS);
})

// TODO: Add GPU commands
// -------------------------------------------------GPU COMMANDS-------------------------------------------------

// OPDEF(gOut, 18, 1, {
//     VAL = GET_VALUE();
//     GOUT(VAL);
// })

/*
OPDEF(ginit, 18, 0, {
    WINDOW = sfRenderWindow_create({WIDTH, HEIGHT, BITS_PER_PIXEL}, WINDOW_NAME, sfClose, &settings);
    sfRenderWindow_setFramerateLimit(WINDOW, MAX_FPS);
})

OPDEF(gCircleInit, 19, 0, {
    CIRCLE = sfCircleShape_create();
    sfCircleShape_setOutlineThickness(CIRCLE, 1);
    sfCircleShape_setOutlineColor(CIRCLE, sfBlack);
    sfCircleShape_setFillColor(CIRCLE, sfWhite);
    sfCircleShape_setRadius(CIRCLE, CIRCLE_RADIUS);
    sfCircleShape_setPosition(CIRCLE, {WIDTH / 2 - CIRCLE_RADIUS, HEIGHT / 2 - CIRCLE_RADIUS});
})

OPDEF(gWindowIsOpen, 20, 0, {
    PUSH((double) sfRenderWindow_isOpen(WINDOW));
})

OPDEF(gWindowClear, 21, 0, {
    sfRenderWindow_clear(WINDOW, sfWhite);
})

OPDEF(gDrawCircle, 22, 0, {
    sfRenderWindow_drawCircleShape(WINDOW, CIRCLE, NULL);
})

OPDEF(gWindowDisplay, 23, 0, {
    sfRenderWindow_display(WINDOW);
})

OPDEF(gClean, 24, 0, {
    sfCircleShape_destroy(CIRCLE);
    sfRenderWindow_destroy(WINDOW);
})
*/

// --------------------------------------------------------------------------------------------------------------
