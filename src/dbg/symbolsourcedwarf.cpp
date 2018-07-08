#include "symbolsourcedwarf.h"


DwarfSymbolSource::DwarfSymbolSource()
{
	isInitialized = false;
}

DwarfSymbolSource::~DwarfSymbolSource()
{
	if(dbg)
	{
		// free the copy of the pointer to the dwarf module object
		Dwarf_Obj_Access_Interface* intf = dbg->de_obj_file;
		DwarfModuleObject* dmo = (DwarfModuleObject*)intf->object;
		free(intf);
	}
	if(m_ModuleObject)
		free(m_ModuleObject);
	if(m_DwarfInterface)
		free(m_DwarfInterface);

}

bool DwarfSymbolSource::initDwarf(PVOID lpBase, duint szFileSize)
{
	// Initialize a dwarf object interface here
	m_ModuleObject = (DwarfModuleObject*)calloc(1, sizeof(DwarfModuleObject));
	if(!m_ModuleObject)
		return false;

	m_ModuleObject->lpFileBase = (PBYTE)lpBase;
	m_ModuleObject->nFileSize = szFileSize;

	m_ModuleObject->pNtHeaders = (PIMAGE_NT_HEADERS)
		(m_ModuleObject->lpFileBase + m_ModuleObject->pDosHeader->e_lfanew);
	m_ModuleObject->pSections = IMAGE_FIRST_SECTION(m_ModuleObject->pNtHeaders);

	m_ModuleObject->pSymbolTable = (PIMAGE_SYMBOL)
		(m_ModuleObject->lpFileBase + m_ModuleObject->pNtHeaders->FileHeader.PointerToSymbolTable);

	if(!m_ModuleObject->pSymbolTable)
		return false;

	m_ModuleObject->pStringTable = (PSTR)&m_ModuleObject->pSymbolTable[m_ModuleObject->pNtHeaders->FileHeader.NumberOfSymbols];
	
	m_DwarfInterface = (Dwarf_Obj_Access_Interface*)calloc(1, sizeof(Dwarf_Obj_Access_Interface));
	if(!m_DwarfInterface)
		return false;

	// set up the callbacks
	// NOTE: Will the methods passed here work as desired for multiple instances of the dwarf class?
	m_DwarfInterface->object = m_ModuleObject;
	m_DwarfInterface->methods = &DwarfPeMethods;

	//init dwarf object
	if(dwarf_object_init(m_DwarfInterface, m_ErrHandler, m_ErrArg, &dbg, &m_LastError) != DW_DLV_OK)
		return false;

	isInitialized = true;
	return true;
}

int PfnGetSectionInfo(void* obj, Dwarf_Half section_index, Dwarf_Obj_Access_Section* return_section, int* err)
{
	DwarfModuleObject* dmo = (DwarfModuleObject*)obj;
	return_section->addr = 0;
	if(section_index == 0)
	{
		return_section->size = 0;
		return_section->name = "";
	}
	else
	{
		PIMAGE_SECTION_HEADER pThisSection = dmo->pSections + section_index - 1;
		if(pThisSection->Misc.VirtualSize < pThisSection->SizeOfRawData)
			return_section->size = pThisSection->Misc.VirtualSize;
		else
			return_section->size = pThisSection->SizeOfRawData;

		return_section->name = (const char*)pThisSection->Name;
		// find real name for section names of mingw compiled binaries
		if(return_section->name[0] == '/')
		{
			return_section->name = &dmo->pStringTable[atoi(&return_section->name[1])];
		}
	}
	return_section->link = 0;
	return_section->entrysize = 0;

	return DW_DLV_OK;
}

Dwarf_Endianness PfnGetByteOrder(void* obj)
{
	return DW_OBJECT_LSB;
}

Dwarf_Small PfnGetLenPointerSize(void* obj)
{
	DwarfModuleObject* dmo = (DwarfModuleObject*)obj;
	PIMAGE_OPTIONAL_HEADER pThisOptionalHeader = &dmo->pNtHeaders->OptionalHeader;

	switch(pThisOptionalHeader->Magic)
	{
	case IMAGE_NT_OPTIONAL_HDR32_MAGIC:
		return 4;
	case IMAGE_NT_OPTIONAL_HDR64_MAGIC:
		return 8;
	default:
		return 0;
	}
}

Dwarf_Unsigned PfnGetSectionCount(void* obj)
{
	DwarfModuleObject* dmo = (DwarfModuleObject*)obj;
	PIMAGE_FILE_HEADER pThisFileHeader = &dmo->pNtHeaders->FileHeader;
	return pThisFileHeader->NumberOfSections + 1;
}

int PfnLoadSection(void* obj, Dwarf_Half section_index, Dwarf_Small ** return_data, int *err)
{
	DwarfModuleObject* dmo = (DwarfModuleObject*)obj;
	if(section_index == 0)
		return DW_DLV_NO_ENTRY;

	PIMAGE_SECTION_HEADER pThisSection = dmo->pSections + section_index - 1;
	*return_data = dmo->lpFileBase + pThisSection->PointerToRawData;
	return DW_DLV_OK;
}
