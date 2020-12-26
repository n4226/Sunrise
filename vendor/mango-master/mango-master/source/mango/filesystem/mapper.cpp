/*
    MANGO Multimedia Development Platform
    Copyright (C) 2012-2020 Twilight Finland 3D Oy Ltd. All rights reserved.
*/
#include <vector>
#include <algorithm>
#include <mango/core/string.hpp>
#include <mango/filesystem/mapper.hpp>
#include <mango/filesystem/path.hpp>

namespace mango {
namespace filesystem {

    // -----------------------------------------------------------------
    // extension registry
    // -----------------------------------------------------------------

#ifdef MANGO_ENABLE_ARCHIVE_ZIP
    AbstractMapper* createMapperZIP(ConstMemory parent, const std::string& password);
#endif
#ifdef MANGO_ENABLE_ARCHIVE_RAR
    AbstractMapper* createMapperRAR(ConstMemory parent, const std::string& password);
#endif
#ifdef MANGO_ENABLE_ARCHIVE_MGX
    AbstractMapper* createMapperMGX(ConstMemory parent, const std::string& password);
#endif

    using CreateMapperFunc = AbstractMapper* (*)(ConstMemory, const std::string&);

    struct MapperExtension
    {
        std::string extension;
        std::string decorated_extension;
        CreateMapperFunc createMapperFunc;

        MapperExtension(const std::string& extension, CreateMapperFunc func)
            : extension(extension)
        {
            decorated_extension = extension + "/";
            createMapperFunc = func;
        }

        ~MapperExtension()
        {
        }

        AbstractMapper* createMapper(ConstMemory memory, const std::string& password) const
        {
            AbstractMapper* mapper = createMapperFunc(memory, password);
            return mapper;
        }
    };

    static std::vector<MapperExtension> g_extensions =
    {
#ifdef MANGO_ENABLE_ARCHIVE_ZIP
        MapperExtension(".zip", createMapperZIP),
        MapperExtension(".cbz", createMapperZIP),
        MapperExtension(".apk", createMapperZIP),
        MapperExtension(".zipx", createMapperZIP),
#endif

#ifdef MANGO_ENABLE_ARCHIVE_MGX
        MapperExtension(".mgx", createMapperMGX),
        MapperExtension(".snitch", createMapperMGX),
#endif

#ifdef MANGO_ENABLE_ARCHIVE_RAR
        MapperExtension(".rar", createMapperRAR),
        MapperExtension(".cbr", createMapperRAR),
#endif
    };

    // -----------------------------------------------------------------
    // FileInfo
    // -----------------------------------------------------------------

    FileInfo::FileInfo()
        : size(0)
        , flags(0)
    {
    }

    FileInfo::FileInfo(const std::string& name, u64 size, u32 flags)
        : size(size)
        , flags(flags)
        , name(name)
    {
    }

    FileInfo::~FileInfo()
    {
    }

    bool FileInfo::isDirectory() const
    {
        return (flags & DIRECTORY) != 0;
    }

    bool FileInfo::isContainer() const
    {
        return (flags & CONTAINER) != 0;
    }

    bool FileInfo::isCompressed() const
    {
        return (flags & COMPRESSED) != 0;
    }

    bool FileInfo::isEncrypted() const
    {
        return (flags & ENCRYPTED) != 0;
    }

    // -----------------------------------------------------------------
    // FileIndex
    // -----------------------------------------------------------------

    void FileIndex::emplace(const std::string& name, u64 size, u32 flags)
    {
        files.emplace_back(name, size, flags);

        const bool isFile = (flags & FileInfo::DIRECTORY) == 0;
        if (isFile && Mapper::isCustomMapper(name))
        {
            // TODO: check that
            // - filename doesn't already end with "/"
            // - flags don't contain FileInfo::CONTAINER

            // file is a container; add it into the index again
            files.emplace_back(name + "/", 0, flags | FileInfo::DIRECTORY | FileInfo::CONTAINER);
        }
    }

    // -----------------------------------------------------------------
    // Mapper
    // -----------------------------------------------------------------

    Mapper::Mapper(const std::string& pathname, const std::string& password)
    {
		// parse and create mappers
        std::string temp = pathname.empty() ? "./" : pathname;
        m_pathname = temp;
        m_basepath = parse(temp, password);
        m_basepath = temp;

#if 0
        printf("# m_basepath: %s\n", m_basepath.c_str());
        printf("# m_pathname: %s\n", m_pathname.c_str());
        printf("\n");
#endif
    }

    Mapper::Mapper(std::shared_ptr<Mapper> mapper, const std::string& pathname, const std::string& password)
    {
        // use parent's mapper
        m_parent_mapper = mapper;
        m_mapper = *mapper;

		// parse and create mappers
        std::string temp = mapper->m_basepath + pathname;
        m_basepath = parse(temp, password);
        m_pathname = mapper->m_pathname + pathname;

#if 0
        printf("# m_basepath: %s\n", m_basepath.c_str());
        printf("# m_pathname: %s\n", m_pathname.c_str());
        printf("\n");
#endif
    }

    Mapper::Mapper(ConstMemory memory, const std::string& extension, const std::string& password)
    {
        // create mapper to raw memory
        m_mapper = createMemoryMapper(memory, extension, password);
    }

    Mapper::~Mapper()
    {
		delete m_parent_memory;
    }

    std::string Mapper::parse(std::string& pathname, const std::string& password)
    {
        std::string filename = pathname;

        for ( ; !filename.empty(); )
        {
            AbstractMapper* mapper = createCustomMapper(pathname, filename, password);
            if (!mapper)
            {
                break;
            }
        }

        return filename;
    }

    AbstractMapper* Mapper::createCustomMapper(std::string& pathname, std::string& filename, const std::string& password)
    {
        std::string f = toLower(filename);

        for (auto &extension : g_extensions)
        {
            size_t n = f.find(extension.decorated_extension);
            if (n != std::string::npos)
            {
                // update string position to skip decorated extension (example: ".zip/")
                n += extension.decorated_extension.length();

                // resolve container filename (example: "foo/bar/data.zip")
                std::string container = filename.substr(0, n - 1);
                std::string postfix = filename.substr(n, std::string::npos);

                AbstractMapper* mapper = nullptr;

                if (!m_mapper)
                {
                    size_t n = container.find_last_of("/\\:");
                    std::string head = container.substr(0, n + 1);
                    container = container.substr(n + 1, std::string::npos);
                    m_mapper = createFileMapper(head);
                }

                if (m_mapper->isFile(container))
                {
                    m_parent_memory = m_mapper->mmap(container);
                    mapper = extension.createMapper(*m_parent_memory, password);
                    m_mappers.emplace_back(mapper);
                    m_mapper = mapper;

                    filename = postfix;
                    pathname = postfix;
                }

                return mapper;
            }
        }

        if (!m_mapper)
        {
            m_mapper = createFileMapper(pathname);
            pathname = "";
        }

        return nullptr;
    }

    AbstractMapper* Mapper::createMemoryMapper(ConstMemory memory, const std::string& extension, const std::string& password)
    {
        std::string f = toLower(extension);

        for (auto &extension : g_extensions)
        {
            size_t n = f.find(extension.decorated_extension);
            if (n == std::string::npos)
            {
                // try again with a non-decorated extension
                n = f.find(extension.extension);
            }

            if (n != std::string::npos)
            {
                // found a container interface; let's create it
                AbstractMapper* mapper = extension.createMapper(memory, password);
                m_mappers.emplace_back(mapper);
                return mapper;
            }
        }

        return nullptr;
    }

    const std::string& Mapper::basepath() const
    {
        return m_basepath;
    }

    const std::string& Mapper::pathname() const
    {
        return m_pathname;
    }

    Mapper::operator AbstractMapper* () const
    {
        return m_mapper;
    }

    bool Mapper::isCustomMapper(const std::string& filename)
    {
        const std::string extension = toLower(getExtension(filename));

        for (auto &node : g_extensions)
        {
            if (extension == node.extension)
            {
                return true;
            }
        }

        return false;
    }

} // namespace filesystem
} // namespace mango
