/* memcpy函数的声明 
 * string.h
 * chinbor
 * 19.5.13
*/

// string.asm中定义
PUBLIC	void*	memcpy(void* p_dst, void* p_src, int size);
PUBLIC	void	memset(void* p_dst, char ch, int size);
PUBLIC	char*	strcpy(char* p_dst, char* p_src);
PUBLIC 	int		strlen(char* p_str);

/**
 * `phys_copy' and `phys_set' are used only in the kernel, where segments
 * are all flat (based on 0). In the meanwhile, currently linear address
 * space is mapped to the identical physical address space. Therefore,
 * a `physical copy' will be as same as a common copy, so does `phys_set'.
 */
#define	phys_copy	memcpy
#define	phys_set	memset
