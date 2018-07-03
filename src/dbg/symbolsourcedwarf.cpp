#include "symbolsourcedwarf.h"


DwarfSymbolSource::DwarfSymbolSource()
{

}

bool DwarfSymbolSource::initDwarf(PVOID lpBase, duint szFileSize)
{
	bool ret = false;
	m_ModuleObject = (DwarfModuleObject*)calloc(1, sizeof(DwarfModuleObject));
	if(!m_ModuleObject)
		return ret;

	m_ModuleObject->lpFileBase = (PBYTE)lpBase;
	m_ModuleObject->nFileSize = szFileSize;
	return false;
}

int PfnGetSectionInfo(void* obj, Dwarf_Half section_index, Dwarf_Obj_Access_Section* return_section, int* err)
{
	return 0;
}

Dwarf_Endianness PfnGetByteOrder(void* obj)
{
	return DW_OBJECT_LSB;
}

Dwarf_Small PfnGetLenPointerSize(void* obj)
{
	return 0;
}

Dwarf_Unsigned PfnGetSectionCount(void* obj)
{
	return 0;
}

int PfnLoadSection(void* obj, Dwarf_Half section_index, Dwarf_Small ** return_data, int *err)
{
	return 0;
}
