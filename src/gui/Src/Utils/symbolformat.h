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
//static SymbolFlags SYM_STRING_NOLABEL = (SYM_STRING | SYM_NOLABEL);
//static SymbolFlags SYM_STRING_NOADDR = (SYM_STRING | SYM_NOADDR);



/*
 * SYM_DEFAULT =  'addr <module.label>' if module/label exist else its addr
 * SYM_NOMODULE = 'addr <label>'
 * SYM_NOLABEL = 'addr <module (.addr?) >'
 * SYM_ADDRONLY = 'addr'
 * SYM_NOADDR = '<module.label>' only, if module doesnt exist '<addr.label>',
 * SYM_STRING = 'addr string', if SYM_NOADDR then 'string' else return SYM_NOADDR behavior
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

    if(flags & SYM_ADDRNOEXTEND)
        addrText = ToHexString(addr);
    else
        addrText = ToPtrString(addr);

    //if module or label is not available just return addr
    if((!bHasModule && !bHasLabel) || (flags & SYM_ADDRONLY) || !(flags & SYM_STRING))
        return addrText;

    // handle special case for strings
    if(flags & SYM_STRING)
    {
        //find possible combinations
        bool bNoLabelFlag = (flags & SYM_NOLABEL);
        bool bNoAddrFlag = (flags & SYM_NOADDR);
        bool bNoModuleFlag = (flags & SYM_NOMODULE);
        QString prt = QString("NoLabel: %1\nNoAddr: %2\nNoModule: %3\n").arg(bNoLabelFlag).
                                                                       arg(bNoAddrFlag).
                                                                       arg(bNoModuleFlag);
        GuiAddLogMessage(prt.toUtf8().constData());

        QString finalText = "";
        if(!bNoAddrFlag)
            finalText = addrText + " ";

        // if a string exists, we only want to return that as its the priority
        if(bHasString)
        {
            GuiAddLogMessage("In hasString\n");
            if(bNoAddrFlag)
            {
                finalText = stringText;
            }
            else
            {
                finalText += stringText;
                //GuiAddLogMessage(finalText.toUtf8().constData());
            }
            return finalText;
        }
        // if module and label exist, check for flags to omit each
        /*
        else if(bHasLabel && bHasModule)
        {
            GuiAddLogMessage("No String, check label and module\n");
            if(bNoLabelFlag)
            {
                GuiAddLogMessage("bNoLabelFalg\n");
                // no label, module.addr
                finalText = QString("%1.%2").arg(moduleText).arg(addrText);
            }
            else if (bNoModuleFlag)
            {
                GuiAddLogMessage("bNoModuleFlag\n");
                // no module. addr <label>
                finalText += QString("<%1>").arg(labelText);
            }
            else // addr <module.label>
                finalText += QString("<%1.%2>").arg(moduleText).arg(labelText);
            GuiAddLogMessage("Returning in haslabel + model\n");
            return finalText;
        }
        */

        else if(bHasModule) //module.addr
        {
            if(!bNoLabelFlag && bHasLabel)
                finalText = QString("%1.%2").arg(moduleText).arg(labelText);
            else
                finalText = QString("%1.%2").arg(moduleText).arg(addrText);
        }
        else if(bHasLabel) //<label>
        {
            if(!bNoLabelFlag)
                finalText = QString("<%1>").arg(labelText);
            else
                finalText = addrText;
        }
        else
        {
            GuiAddLogMessage("Character check\n");
            //no string but lets see if there are printable characters
            if(addr == (addr & 0xFF))
            {
                QChar c = QChar((char)addr);
                if(c.isPrint() || c.isSpace())
                    finalText += QString(" '%1'").arg(EscapeCh(c));
            }
            else if(addr == (addr & 0xFFF)) //UNICODE?
            {
                QChar c = QChar((ushort)addr);
                if(c.isPrint() || c.isSpace())
                    finalText += QString(" L'%1'").arg(EscapeCh(c));
            }
        }
        return finalText;
    }

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

    // no label
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

    // no module
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

    // module only
    if(flags & SYM_MODULEONLY)
    {
        //if no module text, return addr
        if(!bHasModule)
            return addrText;
        return QString("%1").arg(moduleText);
    }

    // label only
    if(flags & SYM_LABELONLY)
    {
        if(!bHasLabel)
            return addrText;
        return QString("<%1>").arg(labelText);
    }

    // module with address
    if(flags & SYM_MODULEWITHADDR)
    {
        if(!bHasModule)
            return addrText;
        return QString("%1.%2").arg(moduleText, addrText);
    }

    return addrText;
}

#endif // SYMBOLFORMAT_H
