/* Copyright (C) 2026 The MiOne contributors
 * SPDX-License-Identifier: Apache-2.0
 * The JB sensor blob imports these no-op VectorImpl vtable tail slots.
 * Android M keeps the storage layout and six real virtual operations but
 * removes the eight reserved methods. Do not duplicate libutils classes.
 */
void reserved_vector_1(void *self) __asm__("_ZN7android10VectorImpl19reservedVectorImpl1Ev");
void reserved_vector_1(void *self) { (void)self; }
void reserved_vector_2(void *self) __asm__("_ZN7android10VectorImpl19reservedVectorImpl2Ev");
void reserved_vector_2(void *self) { (void)self; }
void reserved_vector_3(void *self) __asm__("_ZN7android10VectorImpl19reservedVectorImpl3Ev");
void reserved_vector_3(void *self) { (void)self; }
void reserved_vector_4(void *self) __asm__("_ZN7android10VectorImpl19reservedVectorImpl4Ev");
void reserved_vector_4(void *self) { (void)self; }
void reserved_vector_5(void *self) __asm__("_ZN7android10VectorImpl19reservedVectorImpl5Ev");
void reserved_vector_5(void *self) { (void)self; }
void reserved_vector_6(void *self) __asm__("_ZN7android10VectorImpl19reservedVectorImpl6Ev");
void reserved_vector_6(void *self) { (void)self; }
void reserved_vector_7(void *self) __asm__("_ZN7android10VectorImpl19reservedVectorImpl7Ev");
void reserved_vector_7(void *self) { (void)self; }
void reserved_vector_8(void *self) __asm__("_ZN7android10VectorImpl19reservedVectorImpl8Ev");
void reserved_vector_8(void *self) { (void)self; }
