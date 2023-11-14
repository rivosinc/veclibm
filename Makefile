CC=/opt/riscv/bin/clang++ \
  --sysroot=/opt/riscv/sysroot \
  -target riscv64-unknown-linux-gnu \
  -march=rv64gcv \
  --gcc-toolchain=/opt/riscv/ \
  -isystem/opt/riscv/riscv64-unknown-linux-gnu/include/c++/12.1.0/riscv64-unknown-linux-gnu \
  -isystem/opt/riscv/riscv64-unknown-linux-gnu/include/c++/12.1.0/ \
  -I./include/
LD=/opt/riscv/bin/clang++ \
  --sysroot=/opt/riscv/sysroot\
  -target riscv64-unknown-linux-gnu \
  -march=rv64gcv \
  --gcc-toolchain=/opt/riscv/ \
  -static 
CFLAGS = -O2
OBJECTS = test_funcs.o test_infra.o \
	  rvvlm_expD.o rvvlm_expD_tbl.o

.c.o:
	$(CC) -c $(CFLAGS) $<
 
test_funcs: $(OBJECTS)
	$(LD) -o $@.r64 $(OBJECTS) $(LDFLAGS)

try_EC: try_EC.o
	$(LD) -o $@.r64 $@.o $(LDFLAGS)


#test_funcs.c: test_funcs.h test_infra.h types.h

#test_infra.o: test_infra.h types.h

clean:
	rm -f *.r64 *.o
