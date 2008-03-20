/***************************************************************************
   Copyright 2006 David Nolden <david.nolden.kdevelop@art-master.de>
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef ENVIRONMENTMANAGER_H
#define ENVIRONMENTMANAGER_H

#include <map>
#include <QtCore/qglobal.h>
#ifdef Q_CC_MSVC
    #include <hash_map>
    #include <hash_set>
#else
    #include <ext/hash_set>
    #include <ext/hash_map>
#endif

#include "hashedstringset.h"
#include <QtCore/QDateTime>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QMutex>

#include <kurl.h>
#include <ksharedptr.h>

#include <parsingenvironment.h>
#include <editorintegrator.h>
#include <iproblem.h>

#include "cppduchainexport.h"
#include "macroset.h"
#include "cachemanager.h"
#include "setrepository.h"

struct HashedStringHash {
  uint operator() (const KDevelop::HashedString& str) const {
    return str.hash();
  }
  
  #ifdef Q_CC_MSVC
  bool operator() (const KDevelop::HashedString& lhs, const KDevelop::HashedString& rhs) const {
    return lhs < rhs;
  }
    enum
		{	// parameters for hash table
		bucket_size = 4,	// 0 < bucket_size
		min_buckets = 8};	// min_buckets = 2 ^^ N, 0 < N
  #endif
};

/**
 * The environment-manager helps achieving right representation of the way c++ works:
 * When a file is processed by the preprocessor, the same file may create totally
 * different results depending on the defined macros. Think for example of header-
 * guards.
 *
 * Now when one file includes another, we want to know whether there already
 * is a readily parsed du-context for the file that WOULD BE CREATED if it was
 * preprocessed under the current environment of macros.
 *
 * The environment-manager is there to answer that question:
 * EnvironmentFile collects all information about the context a file was parsed in,
 * the macros used, the words contained in a file that can be influenced by macros,
 * and the defined macros.
 *
 * The environment-manager is able to match that information agains a current parsing-environment
 * to see whether preprocessing the file would yield the same result as an already stored
 * run.
 *
 * If the result would be different, the file will be re-preprocessed, parsed, and imported.
 * Else the set of defined macros is taken from the stored EnvironmentFile,
 * and the already available du-context will be imported. The result: correct behavior, perfectly working header-guards, no missing macros, intelligent reparsing of changed headers, ...
 *
 * There is only one assumption made that needs to be fulfilled to be correct:
 * - Headers should be included at the top of the file
 * */

/**
 * Simplified matching:
 * Consider this case:
----- File 3.h
#ifndef HAD_3
#define HAD_3
int i;
#endif

----- File 2.h
#include "3.h"
int independent;

----- File test_1.h
#include "2.h"

----- File test_2.h
#include "3.h"
#include "2.h"

----- end

2.h and 3.h both depend on the macro HAD_3. In test_2.h, 3.h is included before 3.h, and when 2.h is included in the next line,
it needs to be completeley re-parsed, because 2.h depends on the macro HAD_3, and that macro is different(it is set) then when it was parsed
in test_1.h.

In practice this is very problematic, because it leads to massive multiple parsing of everything, which costs a lot of resources.
To solve this, "Simplified matching" can be used(it is enabled in CppLanguageSupport).
With simplified matching enabled, every file is always represented by at least 2 parts:
One part for all the headers at the top(we will call it header-section), and one part for the real content below the headers.
From the header-section, we create a proxy-context, that only stores the environment-matching information, and that imports the next created content-context.
The content-context starts right behind the header-context, and its environment-matching information only represents the real content.
The content-context can then be re-used many times, as long as the environment matches, while many different proxy-contexts will be created that represent
different constellations of macro-dependences across included headers.

The downside of this approach:
- Many different includes may be added to the content-context, coming from different proxy-contexts. This is not 100% correct visibility-wise.
- This only works perfectly if the includes are at the top, within one block. If this is not the case, the behavior will partially become like without simplified matching.
The good things:
- The general code-completion should be exactly as good as without simplified matching.
* */

namespace rpp {
  class pp_macro;
  class Environment;
}

namespace Cpp {

typedef Utils::SetRepository<KDevelop::HashedString, HashedStringHash>::Iterator StringSetIterator;
typedef Utils::SetRepository<KDevelop::HashedString, HashedStringHash> StringSetRepository;

typedef Utils::SetRepository<rpp::pp_macro, rpp::pp_macro::CompleteHash>::Iterator MacroRepositoryIterator;
typedef Utils::SetRepository<rpp::pp_macro, rpp::pp_macro::CompleteHash> MacroRepository;

class EnvironmentManager;
class MacroSet;

class KDEVCPPDUCHAIN_EXPORT EnvironmentFile : public CacheNode, public KDevelop::ParsingEnvironmentFile {
  public:
    ///@todo Respect changing include-paths: Check if the included files are still the same(maybe new files are found that were not found before)
    EnvironmentFile( const KDevelop::HashedString& url, EnvironmentManager* manager );

    void addStrings( const std::set<Utils::BasicSetRepository::Index>& strings );

    ///If there previously was a macro defined of the same name, it must be given through previousOfSameName, else it can be zero.
    void addDefinedMacro( const rpp::pp_macro& macro, const rpp::pp_macro* previousOfSameName );

    ///the given macro will only make it into usedMacros() if it was not defined in this file
    void usingMacro( const rpp::pp_macro& macro );

    void addIncludeFile( const KDevelop::HashedString& file, const KDevelop::ModificationRevision& modificationTime );

//     inline bool hasString( const KDevelop::HashedString& string ) const {
//       return m_strings[string];
//     }

    ///Returns the set of all strings that can affect this file from outside.
    Utils::Set strings() const;
    
    //The parameter should be a EnvironmentFile that was lexed AFTER the content of this file
    void merge( const EnvironmentFile& file );

    bool operator <  ( const EnvironmentFile& rhs ) const {
      return m_url < rhs.m_url;
    }

    size_t hash() const;

    virtual KDevelop::IdentifiedFile identity() const;

    KDevelop::HashedString url() const;

    void setModificationRevision( const KDevelop::ModificationRevision& rev ) ;
    
    virtual KDevelop::ModificationRevision modificationRevision() const;
    
    /**Set of all files with absolute paths, including those included indirectly
     *
     * This by definition also includes this file, so when the count is 1,
     * no other files were included.
     *
     * */
    //const HashedStringSet& includeFiles() const;

    void addMissingIncludeFile(const KDevelop::HashedString& file);
    const StringSetRepository::LazySet& missingIncludeFiles() const;

    void clearMissingIncludeFiles();
  
    ///Set of all defined macros, including those of all deeper included files
    const MacroRepository::LazySet& definedMacros() const;

    ///Set of all macros used from outside, including those used in deeper included files
    const MacroRepository::LazySet& usedMacros() const;

    const StringSetRepository::LazySet& usedMacroNames() const;
    
    const StringSetRepository::LazySet& definedMacroNames() const;
  
    ///Should contain a modification-time for each included-file
    const QMap<KDevelop::HashedString, KDevelop::ModificationRevision>& allModificationTimes() const;

    ///Should return the include-paths that were used while parsing this file(as used/found in CppLanguageSupport)
    const QList<KDevelop::HashedString>& includePaths() const;
    void setIncludePaths( const QList<KDevelop::HashedString>& paths );

    /**
     * The identity-value usually only depends on the content of the environment-information. This can be used to separate environments that have the same content.
     * For example a content-environment from a proxy-environment.
     * */
    void setIdentityOffset(uint offset);
    uint identityOffset() const;

    ///Set the first line of actual content, behind includes etc.
    void setContentStartLine(int line);
    int contentStartLine() const;

  private:
    virtual int type() const;

    friend class EnvironmentManager;
    uint m_identityOffset;
    QList<KDevelop::HashedString> m_includePaths;
    KDevelop::HashedString m_url;
    KDevelop::ModificationRevision m_modificationTime;
    Utils::Set m_strings; //Set of all strings that can be affected by macros from outside
    Cpp::StringSetRepository::LazySet m_includeFiles; //Set of all files with absolute paths
    Cpp::StringSetRepository::LazySet m_missingIncludeFiles; //Set of relative file-names of missing includes
    Cpp::MacroRepository::LazySet m_usedMacros; //Set of all macros that were used, and were defined outside of this file
    Cpp::StringSetRepository::LazySet m_usedMacroNames; //Set the names of all used macros
    Cpp::MacroRepository::LazySet m_definedMacros; //Set of all macros that were defined while lexing this file
    Cpp::StringSetRepository::LazySet m_definedMacroNames;
    QMap<KDevelop::HashedString, KDevelop::ModificationRevision>  m_allModificationTimes;
    mutable int m_contentStartLine; //Line-number where the actual content starts. Needs to be kept current when edited
    /*
    Needed data:
    1. Set of all strings that appear in this file(For memory-reasons they should be taken from a global string-repository, because many will be the same)
    2. Set of all macros that were defined outside of, but affected the file

    Algorithm:
      Iterate over all available macros, and check whether they affect the file. If it does, make sure that the macro is in the macro-set and has the same body.
      If the check fails: We need to reparse.
    */
};

typedef KSharedPtr<EnvironmentFile>  EnvironmentFilePointer;

struct KDEVCPPDUCHAIN_EXPORT EnvironmentFilePointerCompare {
  bool operator() ( const EnvironmentFilePointer& lhs, const EnvironmentFilePointer& rhs ) const {
    return (*lhs) < (*rhs );
  }
};

class KDEVCPPDUCHAIN_EXPORT EnvironmentManager : public CacheManager, public KDevelop::ParsingEnvironmentManager {
  public:
    EnvironmentManager();
    virtual ~EnvironmentManager();

    ///No lock needs to be acquired for reading, only for writing.
    static QMutex m_repositoryMutex;
    static StringSetRepository m_stringRepository;
    static MacroRepository m_macroRepository;

    /**
     * Use this to save memory. Whenever retrieving a string from a parsed file, first unify it.
     * */
    static const KDevelop::HashedString& unifyString( const KDevelop::HashedString& str ) {
      QMutexLocker lock(&m_repositoryMutex);
      #ifdef Q_CC_MSVC
        stdext::hash_set<KDevelop::HashedString>::const_iterator it = m_totalStringSet.find( str );
      #else
        __gnu_cxx::hash_set<KDevelop::HashedString>::const_iterator it = m_totalStringSet.find( str );
      #endif
      if( it != m_totalStringSet.end() ) {
        return *it;
      } else {
        m_totalStringSet.insert( str );
        return str;
      }
    }

    virtual void saveMemory();

    //Overridden from ParsingEnvironmentManager
    virtual void clear();

    ///Add a new file to the manager
    virtual void addFile( KDevelop::ParsingEnvironmentFile* file );
    ///Remove a file from the manager
    virtual void removeFile( KDevelop::ParsingEnvironmentFile* file );

    ///See the comment about simplified matching at the top
    void setSimplifiedMatching(bool simplified);
    bool isSimplifiedMatching() const;
    
    /**
     * Search for the availability of a file parsed in a given environment
     * */
    virtual KDevelop::ParsingEnvironmentFile* find( const KDevelop::HashedString& url, const KDevelop::ParsingEnvironment* environment, KDevelop::ParsingEnvironmentFileAcceptor* accepter );

  private:
    virtual int type() const;
    ///before this can be called, initFileModificationCache should be called once
    QDateTime fileModificationTimeCached( const KDevelop::HashedString& fileName ) const;
    void initFileModificationCache();
    virtual void erase( const CacheNode* node );
    bool hasSourceChanged( const EnvironmentFile& file ) const;///Returns true if the file itself, or any of its dependencies was modified.

    ///Returns zero if no fitting file is available for the given Environment
    EnvironmentFilePointer lexedFile( const KDevelop::HashedString& fileName, const rpp::Environment* environment, KDevelop::ParsingEnvironmentFileAcceptor* accepter );
    void addEnvironmentFile( const EnvironmentFilePointer& file );
    void removeEnvironmentFile( const EnvironmentFilePointer& file );

    //typedef __gnu_cxx::hash_multimap<KDevelop::HashedString, EnvironmentFilePointer> EnvironmentFileMap;
    typedef std::multimap<KDevelop::HashedString, EnvironmentFilePointer> EnvironmentFileMap;
    EnvironmentFileMap m_files;
    #ifdef Q_CC_MSVC
        static stdext::hash_set<KDevelop::HashedString> m_totalStringSet; ///This is used to reduce memory-usage: Most strings appear again and again. Because QString is reference-counted, this set contains a unique copy of each string to used for each appearance of the string
    #else
        static __gnu_cxx::hash_set<KDevelop::HashedString> m_totalStringSet; ///This is used to reduce memory-usage: Most strings appear again and again. Because QString is reference-counted, this set contains a unique copy of each string to used for each appearance of the string
    #endif
    struct FileModificationCache {
      QDateTime m_readTime;
      QDateTime m_modificationTime;
    };
    #ifdef Q_CC_MSVC
        typedef stdext::hash_map<KDevelop::HashedString, FileModificationCache> FileModificationMap;
    #else    
        typedef __gnu_cxx::hash_map<KDevelop::HashedString, FileModificationCache> FileModificationMap;
    #endif
    mutable FileModificationMap m_fileModificationCache;
    QDateTime m_currentDateTime;
    bool m_simplifiedMatching;
};

}

#endif
