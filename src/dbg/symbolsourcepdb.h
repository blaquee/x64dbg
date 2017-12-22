#ifndef _SYMBOLSOURCEPDB_H_
#define _SYMBOLSOURCEPDB_H_

#include "pdbdiafile.h"
#include "symbolsourcebase.h"
#include "sortedlru.h"

#include <thread>
#include <atomic>
#include <mutex>

class SpinLock
{
private:
    std::atomic_flag _locked;

public:
    SpinLock() { _locked.clear(); }
    void lock()
    {
        while(_locked.test_and_set(std::memory_order_acquire)) { ; }
    }
    void unlock()
    {
        _locked.clear(std::memory_order_release);
    }
};

class ScopedSpinLock
{
private:
    SpinLock & _lock;
public:
    ScopedSpinLock(SpinLock & lock) : _lock(lock) { _lock.lock(); }
    ~ScopedSpinLock() { _lock.unlock(); }
};

class SymbolSourcePDB : public SymbolSourceBase
{
    struct CachedLineInfo
    {
        uint32 rva;
        uint32 lineNumber;
        uint32 sourceFileIdx;
    };

    struct ScopedDecrement
    {
    private:
        std::atomic<duint> & _counter;
    public:
        ScopedDecrement(std::atomic<duint> & counter) : _counter(counter) {}
        ~ScopedDecrement() { _counter--; }
    };

private:
    PDBDiaFile _pdb;

    //All the symbol data, sorted by name
    std::vector<SymbolInfo> _symNames;
    //Symbol addresses to index in _symNames (TODO: refactor to std::vector)
    std::map<duint, size_t> _symAddrs;

    //std::map<duint, SymbolInfo> _sym;
    std::map<duint, CachedLineInfo> _lines;
    std::thread _symbolsThread;
    std::thread _sourceLinesThread;
    std::atomic<bool> _requiresShutdown;
    std::atomic<duint> _loadCounter;
    std::vector<String> _sourceFiles;
    duint _imageBase;
    duint _imageSize;
    SpinLock _lockSymbols;
    SpinLock _lockLines;

public:
    static bool isLibraryAvailable()
    {
        return PDBDiaFile::initLibrary();
    }

public:
    SymbolSourcePDB();

    virtual ~SymbolSourcePDB() override;

    virtual bool isOpen() const override;

    virtual bool isLoading() const override;

    virtual bool cancelLoading() override;

    virtual bool findSymbolExact(duint rva, SymbolInfo & symInfo) override;

    virtual bool findSymbolExactOrLower(duint rva, SymbolInfo & symInfo) override;

    virtual void enumSymbols(const CbEnumSymbol & cbEnum) override;

    virtual bool findSourceLineInfo(duint rva, LineInfo & lineInfo) override;

    virtual bool findSymbolByName(const std::string & name, SymbolInfo & symInfo, bool caseSensitive) override;

    virtual bool findSymbolsByPrefix(const std::string & prefix, std::vector<SymbolInfo> & symbols, bool caseSensitive) override;

public:
    bool loadPDB(const std::string & path, duint imageBase, duint imageSize);

private:
    void loadPDBAsync();
    bool loadSymbolsAsync(String path);
    bool loadSourceLinesAsync(String path);
};

#endif // _SYMBOLSOURCEPDB_H_
