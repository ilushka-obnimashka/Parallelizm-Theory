CFLAG ?= -fopenmp
MATRIX_SIZE ?= 20000
NTHREADS ?= 1
BUILD_DIR = build

$(BUILD_DIR)/task1: task1.c FORCE
	mkdir -p $(BUILD_DIR)
	gcc -DMATRIX_SIZE=$(MATRIX_SIZE) -DNTHREADS=$(NTHREADS) $(CFLAG) -o $@ $<

$(BUILD_DIR)/task2: task2.c FORCE
	mkdir -p $(BUILD_DIR)
	gcc -o $@ $< -DNTHREADS=$(NTHREADS) $(CFLAG) -lm


$(BUILD_DIR)/task3_each_section: task3_metod_1.cpp FORCE
	mkdir -p $(BUILD_DIR)
	g++ -DMATRIX_SIZE=$(MATRIX_SIZE) -DNTHREADS=$(NTHREADS) $(CFLAG) -o $@ $<

$(BUILD_DIR)/task3_one_section: task3_metod_2.cpp FORCE
	mkdir -p $(BUILD_DIR)
	g++ -DMATRIX_SIZE=$(MATRIX_SIZE) -DNTHREADS=$(NTHREADS) $(CFLAG) -o $@ $<	

FORCE:

