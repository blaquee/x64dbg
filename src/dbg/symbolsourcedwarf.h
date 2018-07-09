#ifndef _DWARF_PE_H
#define _DWARF_PE_H


#include <_global.h>
#include <windows.h>
#include <tchar.h>
#include <string.h>

#include <vector>
#include "symbolsourcebase.h"

#include <libdwarf/dwarf_incl.h>

#ifdef __cplusplus
extern "C" {
#endif

	// Information about a PE module for use by the Dwarf Object Interface Methods
	typedef struct
	{
		HANDLE hFileMapping;
		SIZE_T nFileSize;
		union {
			PBYTE lpFileBase; // real base
			PIMAGE_DOS_HEADER pDosHeader;
		};
		PIMAGE_NT_HEADERS pNtHeaders;
		PIMAGE_SECTION_HEADER pSections;
		PIMAGE_SYMBOL pSymbolTable;
		PSTR pStringTable;
	}DwarfModuleObject;


	// dwarf object helper functions
	// These have to be global because Dwarf lib expects these callbacks for
	// Dwarf_Obj_Access_Methods
	int PfnGetSectionInfo(void* obj, Dwarf_Half section_index,
		Dwarf_Obj_Access_Section* return_section, int* err);
	Dwarf_Endianness PfnGetByteOrder(void* obj);
	Dwarf_Small PfnGetLenPointerSize(void* obj);
	Dwarf_Unsigned PfnGetSectionCount(void* obj);
	int PfnLoadSection(void* obj, Dwarf_Half section_index,
		Dwarf_Small ** return_data, int *err);

	//dwarf object methods structure
	const Dwarf_Obj_Access_Methods DwarfPeMethods
	{
		PfnGetSectionInfo,
		PfnGetByteOrder,
		PfnGetLenPointerSize,
		PfnGetLenPointerSize,
		PfnGetSectionCount,
		PfnLoadSection
	};

	class DwarfSymbolSource : public SymbolSourceBase
	{


	private:
		Dwarf_Debug dbg;
		Dwarf_Handler m_ErrHandler;
		Dwarf_Ptr m_ErrArg;
		Dwarf_Error m_LastError;
		Dwarf_Obj_Access_Interface* m_DwarfInterface;
		DwarfModuleObject* m_ModuleObject;
		bool isDwarf;
		bool isInitialized;


	private:
		std::vector<SymbolInfo> _symData;
		std::string _path;
		std::vector<String> _sourceFiles;
		duint _imageSize;
		
	public:
		DwarfSymbolSource();
		//DwarfPE();
		virtual ~DwarfSymbolSource();

		virtual bool isOpen() const override;
		virtual bool findSymbolExact(duint rva, SymbolInfo & symInfo) override;
		virtual bool findSymbolExactOrLower(duint rva, SymbolInfo & symInfo) override;
		virtual void enumSymbols(const CbEnumSymbol & cbEnum) override;

		virtual bool findSourceLineInfo(duint rva, LineInfo & lineInfo) override;
		virtual bool findSymbolByName(const std::string & name, SymbolInfo & symInfo, bool caseSensitive) override;

	public:
		// Get lpBase and szFileSize from MODINFO in modules.cpp
		bool initDwarf(PVOID lpBase, duint szFileSize); // initializes the dwarf library interface
		DwarfModuleObject* getDwarfModule(); // to retun this classes dwarf module object



	};

#ifdef __cplusplus
}
#endif

#endif