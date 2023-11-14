
// This table is used my different functions of the exp and exponential family
#include <stdint.h>


// The following contains exp(j/64) for j = 0, 1, ..., 63
// We need these values to more than FP64 precision. We do this
// by exploiting fixed-point computation supported by RISC-V
// This table contains round-to-int(2^62 * exp(j/64)) 
constexpr extern int64_t expD_tbl64_fixedpt[64] = {
  0x4000000000000000, 0x40b268f9de0183ba, 0x4166c34c5615d0ec, 0x421d1461d66f2023,
  0x42d561b3e6243d8a, 0x438fb0cb4f468808, 0x444c0740496d4294, 0x450a6abaa4b77ecd,
  0x45cae0f1f545eb73, 0x468d6fadbf2dd4f3, 0x47521cc5a2e6a9e0, 0x4818ee218a3358ee,
  0x48e1e9b9d588e19b, 0x49ad159789f37496, 0x4a7a77d47f7b84b1, 0x4b4a169b900c2d00,
  0x4c1bf828c6dc54b8, 0x4cf022c9905bfd32, 0x4dc69cdceaa72a9c, 0x4e9f6cd3967fdba8,
  0x4f7a993048d088d7, 0x50582887dcb8a7e1, 0x513821818624b40c, 0x521a8ad704f3404f,
  0x52ff6b54d8a89c75, 0x53e6c9da74b29ab5, 0x54d0ad5a753e077c, 0x55bd1cdad49f699c,
  0x56ac1f752150a563, 0x579dbc56b48521ba, 0x5891fac0e95612c8, 0x5988e20954889245,
  0x5a827999fcef3242, 0x5b7ec8f19468bbc9, 0x5c7dd7a3b17dcf75, 0x5d7fad59099f22fe,
  0x5e8451cfac061b5f, 0x5f8bccdb3d398841, 0x6096266533384a2b, 0x61a3666d124bb204,
  0x62b39508aa836d6f, 0x63c6ba6455dcd8ae, 0x64dcdec3371793d1, 0x65f60a7f79393e2e,
  0x6712460a8fc24072, 0x683199ed779592ca, 0x69540ec8f895722d, 0x6a79ad55e7f6fd10,
  0x6ba27e656b4eb57a, 0x6cce8ae13c57ebdb, 0x6dfddbcbed791bab, 0x6f307a412f074892,
  0x70666f76154a7089, 0x719fc4b95f452d29, 0x72dc8373be41a454, 0x741cb5281e25ee34,
  0x75606373ee921c97, 0x76a7980f6cca15c2, 0x77f25ccdee6d7ae6, 0x7940bb9e2cffd89d,
  0x7a92be8a92436616, 0x7be86fb985689ddc, 0x7d41d96db915019d, 0x7e9f06067a4360ba,
};

