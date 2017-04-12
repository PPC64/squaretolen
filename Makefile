.phony: all o3 debug clean test perf

CC=gcc-6
C_FILES=squaretolen.c main.c squaretolen.h
DBG_FLAGS=-O0 -g
FLAGS=-O3 -Wall
OUT=main
ASM=asm
DBG=dbg
PERF=perf
ASM_FLAG=-D_ASM
PERF_FLAGS=-DTIME
I=100

all: o3 debug

o3: $(OUT) $(ASM).$(OUT)

$(OUT): $(C_FILES)
	$(CC) $^ -o $@ $(FLAGS)

$(ASM).$(OUT): $(C_FILES)
	$(CC) $^ -o $@ $(FLAGS) $(ASM_FLAG)

debug: $(DBG).$(OUT) $(ASM).$(DBG).$(OUT)

$(DBG).$(OUT):$(C_FILES)
	$(CC) $^ -o $@ $(DBG_FLAGS)

$(ASM).$(DBG).$(OUT):$(C_FILES)
	$(CC) $^ -o $@ $(DBG_FLAGS) $(ASM_FLAG)

$(OUT).$(PERF): $(C_FILES)
	$(CC) $^ -o $@ $(FLAGS) $(PERF_FLAGS)

$(ASM).$(OUT).$(PERF): $(C_FILES)
	$(CC) $^ -o $@ $(FLAGS) $(ASM_FLAG) $(PERF_FLAGS)

debug: $(DBG).$(OUT) $(ASM).$(DBG).$(OUT)

perf: $(OUT).$(PERF) $(ASM).$(OUT).$(PERF)
	mkdir -p doc/
	sh plot_results.sh "doc/SquareToLen_perf" $(OUT).$(PERF) $(ASM).$(OUT).$(PERF) $(I)

clean:
	rm -f $(OUT) $(ASM).$(OUT) $(DBG).$(OUT) $(ASM).$(DBG).$(OUT)

test: all
	@echo -n "./$(ASM).$(OUT) ";        bash -c "cmp <(./$(OUT) $(I))        <(./$(ASM).$(OUT) $(I))        > /dev/null"  && echo success || echo failed!
	@echo -n "./$(ASM).$(DBG).$(OUT) "; bash -c "cmp <(./$(DBG).$(OUT) $(I)) <(./$(ASM).$(DBG).$(OUT) $(I)) > /dev/null"  && echo success || echo failed!
