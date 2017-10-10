#ifndef SYMBOLFORMAT_H
#define SYMBOLFORMAT_H

typedef unsigned int SymbolFlags;

static SymbolFlags SYM_DEFAULT          = 1 << 0;
static SymbolFlags SYM_ADDRONLY         = 1 << 1;
static SymbolFlags SYM_NOMODULE         = 1 << 2;
static SymbolFlags SYM_NOLABEL          = 1 << 3;
static SymbolFlags SYM_NOADDR           = 1 << 4;
static SymbolFlags SYM_ADDRNOEXTEND     = 1 << 5;
static SymbolFlags SYM_STRING           = 1 << 6;
static SymbolFlags SYM_MODULEWITHADDR = (SYM_ADDRONLY | SYM_NOLABEL);
static SymbolFlags SYM_MODULEONLY = (SYM_NOLABEL | SYM_NOADDR);
static SymbolFlags SYM_LABELONLY = (SYM_NOMODULE | SYM_NOADDR);



/*
 * SYM_DEFAULT =  'addr <module.label>' if module/label exist else its addr
 * SYM_NOMODULE = 'addr <label>'
 * SYM_NOLABEL = 'addr <module (.addr?) >'
 * SYM_ADDRONLY = 'addr'
 * SYM_NOADDR = '<module.label>' only, if module doesnt exist '<addr.label>',
 * SYM_STRING = 'addr string', if SYM_NOADDR then 'string'
 * if label doesnt exist '<module.addr>'
 * For any combination that requests module or label, if either one does not exist
 * then addr is the default fallback.
 * i.e: if SYM_NOMODULE is used, and label doesn't exist, addr is returned.
 * i.e: if SYM_NOLABEL is used and module doesnt exist, addr is returned.
 * note: <> is used for any string with a label
 * */
static QString getSymbolicName(duint addr, SymbolFlags flags)
{
    char labelText[MAX_LABEL_SIZE] = "";
    char moduleText[MAX_MODULE_SIZE] = "";
    char stringText[MAX_STRING_SIZE] = "";
    bool bHasString = DbgGetStringAt(addr, stringText);
    bool bHasLabel = DbgGetLabelAt(addr, SEG_DEFAULT, labelText);
    bool bHasModule = (DbgGetModuleAt(addr, moduleText) && !QString(labelText).startsWith("JMP.&"));
    QString addrText = "";
    QString finalText = "";

    if(flags & SYM_ADDRNOEXTEND)
        addrText = ToHexString(addr);
    else
        addrText = ToPtrString(addr);

    //if module or label is not available just return addr
    if((!bHasModule && !bHasLabel) || (flags & SYM_ADDRONLY))
        return addrText;

    if(flags & SYM_NOADDR)
    {
        GuiAddLogMessage("SYM_NOADDR\n");
        // we already handled the case of no mod and no label before, so
        // if either one is missing here, we return what is available.
        if(!bHasModule && bHasLabel && !(flags & SYM_NOLABEL))
            return QString("<%1>").arg(labelText);
        else if(!bHasLabel && bHasModule && !(flags & SYM_NOMODULE))
            return QString("%1").arg(moduleText);
        else
            return QString("<%1.%2>").arg(moduleText, labelText);
    }

    //handle strings
    if(flags & SYM_STRING)
    {
        GuiAddLogMessage("SYM_STRING\n");
        if(bHasString && (flags & SYM_NOADDR))
            return QString(stringText);
        else
            return QString("%1 %2").arg(addrText, stringText);
    }

    //If default is used, attempt to use all available information
    if(flags & SYM_DEFAULT)
    {
        GuiAddLogMessage("SYM_DEFAULT\n");
        // addr <module.label>
        if(bHasLabel && bHasModule)
            return QString("%1 <%2.%3>").arg(addrText).arg(moduleText).arg(labelText);
        else
            return addrText;
    }

    if(flags & SYM_NOLABEL)
    {
        GuiAddLogMessage("SYM_NOLABEL\n");
        if(!bHasModule)
        {
            GuiAddLogMessage("No Label No Module\n");
            return addrText;
        }
        return QString("%1 <%2.%3>").arg(addrText, moduleText, addrText);
    }

    if(flags & SYM_NOMODULE)
    {
        GuiAddLogMessage("SYM_NOMODULE\n");
        if(!bHasLabel)
        {
            GuiAddLogMessage("No Label in No Module\n");
            return addrText;
        }
        return QString("%1 <%2>").arg(addrText, labelText);
    }

    if(flags & SYM_MODULEONLY)
    {
        //if no module text, return addr
        if(!bHasModule)
            return addrText;
        return QString("%1").arg(moduleText);
    }

    if(flags & SYM_LABELONLY)
    {
        if(!bHasLabel)
            return addrText;
        return QString("<%1>").arg(labelText);
    }

    if(flags & SYM_MODULEWITHADDR)
    {
        if(!bHasModule)
            return addrText;
        return QString("%1.%2").arg(moduleText, addrText);
    }

    return addrText;
}

#endif // SYMBOLFORMAT_H
