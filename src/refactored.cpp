#include <iostream>
#include <memory>
#include <utility>
#include <fstream>
#include <string>
#include <ctime>



class FileProcessor
{

  public:

    virtual void processFile(const std::string & filePath) = 0;

    virtual ~FileProcessor() noexcept
    { /*empty*/ }

};



class IdentityFileProcessor final : public FileProcessor
{

  public:

    // Performs identity transformation and cannot throw
    void processFile( [[maybe_unused]] const std::string & filePath) noexcept override
    { /* empty */ }

};



class FileEncoder final : public FileProcessor
{

  public:

    void processFile(const std::string & filePath) override
    {
        std::ifstream inp(filePath);
        if(!inp.is_open())
        {
            // FIXME: How should we react? What should we throw?
            throw "Troubles with openning \"" + filePath + "\"";
        }
        std::string fileContent;
        inp >> fileContent;

        encodeFile(fileContent);
    }

    void encodeFile(const std::string & fileContent)
    { std::cout << "encoding file with content: " << fileContent << "\n"; }

};



class FileCompressor final : public FileProcessor
{

  public:

    using param_t = std::string;

  private:

    param_t _param1;
    param_t _param2;

  public:

    FileCompressor(const param_t & param1, const param_t & param2)
            : _param1(param1), _param2(param2)
    { /* empty */ }

    FileCompressor(FileCompressor && another) noexcept(std::is_nothrow_move_constructible<param_t>::value)
            : _param1(""), _param2("")
    {
        std::swap(_param1, another._param1);
        std::swap(_param2, another._param2);
    }

    FileCompressor & operator=(FileCompressor && another) noexcept(std::is_nothrow_move_assignable<param_t>::value)
    {
        std::swap(_param1, another._param1);
        std::swap(_param2, another._param2);
        return (*this);
    }

    void processFile(const std::string & filePath) override
    {
        std::ifstream inp(filePath);
        if(!inp.is_open())
        {
            // FIXME: How should we react? What should we throw?
            throw "Troubles with openning \"" + filePath + "\"";
        }
        std::string fileContent;
        inp >> fileContent;

        compressFile(fileContent);
    }

    void compressFile(const std::string & fileContent)
    {
        std::cout << "compressing file with content: " << fileContent
                  << " using params " << _param1 << " " << _param2 << "\n";
    }

};



class FileEncryptor final : public FileProcessor
{

  private:

    char * _encryptionKey;

  public:

    FileEncryptor(void) : _encryptionKey(nullptr)
    {
        try
        {
            _encryptionKey = new char[16];
        }
        catch(std::bad_alloc & ba_exc)
        {
            // FIXME: What should we do? What should we rethrow?
            throw ba_exc;
        }

        srand(time(NULL));
        for(int i = 0; i < 15; ++i)
            _encryptionKey[i] = rand() % 64 + ' '; // Generate printable key

        _encryptionKey[15] = 0;
    }

    ~FileEncryptor() noexcept
    { clearKey(); }

    void processFile(const std::string & filePath) override
    {
        std::ifstream inp(filePath);
        if(!inp.is_open())
        {
            // FIXME: How should we react? What should we throw?
            throw "Troubles with openning \"" + filePath + "\"";
        }
        std::string fileContent;
        inp >> fileContent;

        encryptFile(fileContent);
    }

    void clearKey(void) noexcept
    {
        std::cout << "Clearing encryption key\n";

        delete _encryptionKey;
    }

    void encryptFile(const std::string & fileContent)
    {
        std::cout << "encrypting file with content: " << fileContent
                  << " using key " << _encryptionKey << "\n";
    }

};

static_assert(std::is_nothrow_move_constructible<FileEncryptor>::value,
              "FileCompressor should be noexcept MoveConstructible");



class FileProcessorFactorySingleton
{

  public:

    static FileProcessorFactorySingleton * getInstance(void)
    {
        static FileProcessorFactorySingleton instance{};
        return &instance;
    }

    std::unique_ptr<FileProcessor> createFileProcessor(const std::string & mode)
    {
        std::unique_ptr<FileProcessor> fp_uptr{nullptr};

        if(mode == "encode")
            fp_uptr.reset(new FileEncoder());
        else if(mode == "identity")
            fp_uptr.reset(new IdentityFileProcessor());
        else if(mode == "compress")
            fp_uptr.reset(new FileCompressor("Hello", "World"));
        else if(mode == "encrypt")
            fp_uptr.reset(new FileEncryptor());

        return fp_uptr;
    }

  private:

    FileProcessorFactorySingleton(void) = default;

};



int main(int argc, char * argv[])
{

    const std::string mode = argv[1];
    std::unique_ptr<FileProcessor> fp {FileProcessorFactorySingleton::getInstance()->createFileProcessor(mode)};

    try
    {
        for(int i = 2; i < argc; ++i)
            fp->processFile(argv[i]);

        return 1;
    }
    catch (...)
    {
        // FIXME: What should we do?
    }

    std::cout << "Something wrong";

    return 0;
}
