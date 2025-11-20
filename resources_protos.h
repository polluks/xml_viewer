/*
 * libresources.a
 */

#ifndef EXEC_EXECBASE_H
   #include <exec/execbase.h>
#endif /* EXEC_EXECBASE_H */

/*
 * Functions
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

APTR LIB_AllocPooledC(struct ExecBase *sysBase, APTR poolHeader, ULONG memSize);
APTR LIB_AllocVecPooledC(struct ExecBase *sysBase, APTR poolHeader, ULONG memSize);

APTR LIB_AllocPooledAlignedC(struct ExecBase *sysBase, APTR poolHeader, ULONG memSize, ULONG memAlign);

#ifndef MEMTRACK

   #define AllocPooledC(_header_, _size_)                   LIB_AllocPooledC(SysBase, _header_, _size_)
   #define AllocVecPooledC(_header_, _size_)                LIB_AllocVecPooledC(SysBase, _header_, _size_)

   #define AllocPooledAlignedC(_header_, _size_, _align_)   LIB_AllocPooledAlignedC(SysBase, _header_, _size_, _align_)

#endif /* MEMTRACK */

/*
 * MemTrack
 */

ULONG LIB_PrepareMemTrack(CONST_STRPTR prefix, ULONG flags);
VOID LIB_EndMemTrack(VOID);

/* Memory */
APTR LIB_AllocMem(ULONG byteSize, ULONG requirements, ULONG type, CONST_STRPTR file, ULONG line, CONST_STRPTR function);
APTR LIB_AllocMemAligned(ULONG byteSize, ULONG requirements, ULONG alignSize, ULONG alignOffset, ULONG type, CONST_STRPTR file, ULONG line, CONST_STRPTR function);
VOID LIB_FreeMem(APTR memoryBlock, ULONG byteSize, ULONG type, CONST_STRPTR file, ULONG line, CONST_STRPTR function);

/* Pools */
APTR LIB_CreatePool(ULONG requirements, ULONG poolSize, ULONG poolThreshold, CONST_STRPTR file, ULONG line, CONST_STRPTR function);
VOID LIB_DeletePool(APTR pool, CONST_STRPTR file, ULONG line, CONST_STRPTR function);
VOID LIB_FlushPool(APTR pool, CONST_STRPTR file, ULONG line, CONST_STRPTR function);

APTR LIB_AllocPooled(APTR pool, ULONG byteSize, ULONG type, CONST_STRPTR file, ULONG line, CONST_STRPTR function);
APTR LIB_AllocPooledAligned(APTR pool, ULONG byteSize, ULONG alignSize, ULONG alignOffset, ULONG type, CONST_STRPTR file, ULONG line, CONST_STRPTR function);
VOID LIB_FreePooled(APTR pool, APTR mem, ULONG byteSize, ULONG type, CONST_STRPTR file, ULONG line, CONST_STRPTR function);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#ifdef MEMTRACK

	#define PrepareMemTrack(_prefix_, _flags_)   LIB_PrepareMemTrack(_prefix_, _flags_)
	#define EndMemTrack                          LIB_EndMemTrack

	/*
	 * Alloc/FreeMem()
	 */

	#undef AllocMem
	#undef FreeMem

   #define AllocMem(_size_, _attrs_)   LIB_AllocMem(_size_, _attrs_, REST_ALLOCMEM, __FILE__, __LINE__, __PRETTY_FUNCTION__)
   #define FreeMem(_memory_, _size_)   LIB_FreeMem(_memory_, _size_, REST_ALLOCMEM, __FILE__, __LINE__, __PRETTY_FUNCTION__)

	/*
	 * Alloc/FreeVec()
	 */

	#undef AllocVec
	#undef FreeVec

   #define AllocVec(_size_, _attrs_)	LIB_AllocMem(_size_, _attrs_, REST_ALLOCVEC, __FILE__, __LINE__, __PRETTY_FUNCTION__)
   #define FreeVec(_memory_)				LIB_FreeMem(_memory_, ~0, REST_ALLOCVEC, __FILE__, __LINE__, __PRETTY_FUNCTION__)

	/*
	 * Alloc/FreeTaskPooled
	 */

	#undef AllocTaskPooled
	#undef FreeTaskPooled

   #define AllocTaskPooled(_size_)				LIB_AllocMem(_size_, ~0, REST_ALLOCTASKP, __FILE__, __LINE__, __PRETTY_FUNCTION__)
   #define FreeTaskPooled(_memory_, _size_)	LIB_FreeMem(_memory_, _size_, REST_ALLOCTASKP, __FILE__, __LINE__, __PRETTY_FUNCTION__)

	/*
	 * Alloc/FreeVecTaskPooled
	 */

	#undef AllocVecTaskPooled
	#undef FreeVecTaskPooled

   #define AllocVecTaskPooled(_size_)	LIB_AllocMem(_size_, ~0, REST_ALLOCTASKP|REST_ALLOCVEC, __FILE__, __LINE__, __PRETTY_FUNCTION__)
   #define FreeVecTaskPooled(_memory_)	LIB_FreeMem(_memory_, ~0, REST_ALLOCTASKP|REST_ALLOCVEC, __FILE__, __LINE__, __PRETTY_FUNCTION__)

	/*
	 * Alloc/FreeVecDMA
	 */

	#undef AllocVecDMA
	#undef FreeVecDMA

   #define AllocVecDMA(_size_, _attrs_)	LIB_AllocMem(_size_, _attrs_, REST_ALLOCVEC|REST_DMA, __FILE__, __LINE__, __PRETTY_FUNCTION__)
   #define FreeVecDMA(_memory_)				LIB_FreeMem(_memory_, ~0, REST_ALLOCVEC|REST_DMA, __FILE__, __LINE__, __PRETTY_FUNCTION__)

	/*
	 * AllocMem/VecAligned
	 */

	#undef AllocMemAligned
	#undef AllocVecAligned

	#define AllocMemAligned(_size_, _attrs_, _align_, _offset_) LIB_AllocMemAligned(_size_, _attrs_, _align_, _offset_, REST_ALLOCMEM|REST_ALIGNED, __FILE__, __LINE__, __PRETTY_FUNCTION__)
	#define AllocVecAligned(_size_, _attrs_, _align_, _offset_) LIB_AllocMemAligned(_size_, _attrs_, _align_, _offset_, REST_ALLOCVEC|REST_ALIGNED, __FILE__, __LINE__, __PRETTY_FUNCTION__)

	/*
	 * Create/DeletePool
	 */

	#undef CreatePool
	#undef DeletePool

	#define CreatePool(_attr_, _size_, _threshold_)	LIB_CreatePool(_attr_, _size_, _threshold_, __FILE__, __LINE__, __PRETTY_FUNCTION__)
	#define DeletePool(_pool_)								LIB_DeletePool(_pool_, __FILE__, __LINE__, __PRETTY_FUNCTION__)

	/*
	 * FlushPool
	 */

	#undef FlushPool

	#define FlushPool(_pool_)								LIB_FlushPool(_pool_, __FILE__, __LINE__, __PRETTY_FUNCTION__)

	/*
	 * Alloc/FreePooled
	 */

	#undef AllocPooled
	#undef FreePooled

	#define AllocPooled(_pool_, _size_)					LIB_AllocPooled(_pool_, _size_, REST_ALLOCMEM|REST_POOLED, __FILE__, __LINE__, __PRETTY_FUNCTION__)
	#define FreePooled(_pool_, _mem_, _size_)			LIB_FreePooled(_pool_, _mem_, _size_, REST_ALLOCMEM|REST_POOLED, __FILE__, __LINE__, __PRETTY_FUNCTION__)

	/*
	 * AllocVec/FreePooled
	 */

	#undef AllocVecPooled
	#undef FreeVecPooled

	#define AllocVecPooled(_pool_, _size_)		LIB_AllocPooled(_pool_, _size_, REST_ALLOCVEC|REST_POOLED, __FILE__, __LINE__, __PRETTY_FUNCTION__)
	#define FreeVecPooled(_pool_, _mem_)		LIB_FreePooled(_pool_, _mem_, ~0, REST_ALLOCVEC|REST_POOLED, __FILE__, __LINE__, __PRETTY_FUNCTION__)

	/*
	 * AllocPooledAligned
	 */

	#undef AllocPooledAligned

	#define AllocPooledAligned(_pool_, _size_, _align_, _offset_)	LIB_AllocPooledAligned(_pool_, _size_, _align_, _offset_, REST_ALLOCVEC|REST_POOLED|REST_ALIGNED, __FILE__, __LINE__, __PRETTY_FUNCTION__)

	/*
	 * AllocPooled (Clear)
	 */

	#undef AllocPooledC
	#undef AllocVecPooledC
	#undef AllocPooledAlignedC

	#define AllocPooledC(_pool_, _size_)									LIB_AllocPooled(_pool_, _size_, REST_ALLOCMEM|REST_POOLED|REST_CLEAR, __FILE__, __LINE__, __PRETTY_FUNCTION__)
	#define AllocVecPooledC(_pool_, _size_)								LIB_AllocPooled(_pool_, _size_, REST_ALLOCVEC|REST_POOLED|REST_CLEAR, __FILE__, __LINE__, __PRETTY_FUNCTION__)
	#define AllocPooledAlignedC(_pool_, _size_, _align_, _offset_)	LIB_AllocPooledAligned(_pool_, _size_, _align_, _offset_, REST_ALLOCVEC|REST_POOLED|REST_ALIGNED|REST_CLEAR, __FILE__, __LINE__, __PRETTY_FUNCTION__)

	/*
	 * Ansi
	 */

	#undef malloc
	#undef calloc
	#undef free

	#define malloc(_size_)				LIB_AllocMem(_size_, ~0, REST_ANSI, __FILE__, __LINE__, __PRETTY_FUNCTION__)
	#define calloc(_size_, _esize_)	LIB_AllocMem(_size_ * _esize_, ~0, REST_ANSI|REST_CLEAR, __FILE__, __LINE__, __PRETTY_FUNCTION__)
	#define free(_memory_)				LIB_FreeMem(_memory_, ~0, REST_ANSI, __FILE__, __LINE__, __PRETTY_FUNCTION__)

#else

	#define PrepareMemTrack(_prefix_, _flags_)   (RETURN_OK)
	#define EndMemTrack()                        ;

#endif /* MEMTRACK */
