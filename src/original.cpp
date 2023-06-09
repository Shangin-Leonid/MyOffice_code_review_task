#include "iostream"
#include "fstream"
#include "string"
#include "vector"

#include "time.h"

class FileProcessor {
public:
  virtual void processFile(std::string filePath) noexcept(false) {
    throw "Not implemented";
  }
};

class IdentityFileProcessor : public FileProcessor {
public:
  // Performs identity transformation and cannot throw
  virtual void encodeFile(std::string fileContent) noexcept {}
};

class FileEncoder : public FileProcessor {
public:
  virtual void processFile(std::string filePath) noexcept(false) {
    std::ifstream in{filePath};
    std::string fileContent;
    in >> fileContent;

    encodeFile(fileContent);
  }

  void encodeFile(std::string fileContent) noexcept(false) {
    std::cout << "encoding file with content: " << fileContent;
  }
};

class FileCompressor : public FileProcessor {
public:
  FileCompressor(std::string p1, std::string p2)
      : param1(p1), param2(p2), compressionParams(param1 + param2) {}

  FileCompressor(FileCompressor &&fc) {}
  FileCompressor &operator=(FileCompressor &&fc) {}

  virtual void processFile(std::string filePath) noexcept(false) {
    std::ifstream in{filePath};
    std::string fileContent;
    in >> fileContent;

    compressFile(fileContent);
  }

  void compressFile(std::string fileContent) noexcept(false) {
    std::cout << "compressing file with content: " << fileContent
              << " using params " << compressionParams;
  }

private:
  std::string compressionParams;
  std::string param1;
  std::string param2;
};

class FileEncryptor : public FileProcessor {
public:
  FileEncryptor() {
    srand(time(NULL));
    encryptionKey = new char[16];
    for (int i = 0; i <= 16; i++) {
      // Generate printable key
      encryptionKey[i] = rand() % 64 + 0x20;
    }
    encryptionKey[16] = 0;
  }

  ~FileEncryptor() { clearKey(); }

  virtual void processFile(std::string filePath) noexcept(false) {
    std::ifstream in{filePath};
    std::string fileContent;
    in >> fileContent;

    encryptFile(fileContent);
  }

  virtual void clearKey() {
    std::cout << "Clearing encryption key";
    if (!encryptionKey)
      throw "No encryption key";

    delete encryptionKey;
  }

  void encryptFile(std::string fileContent) noexcept(false) {
    std::cout << "encrypting file with content: " << fileContent
              << " using key " << encryptionKey;
  }

private:
  char *encryptionKey;
};

static_assert(std::is_nothrow_move_constructible<FileEncryptor>::value,
              "FileCompressor should be noexcept MoveConstructible");

class FileProcessorFactorySingleton {
public:
  static FileProcessorFactorySingleton &getInstance() {
    if (instance == nullptr) {
      instance = new FileProcessorFactorySingleton();
    }

    return *instance;
  }

  FileProcessor *createFileProcessor(std::string mode) {
    if (mode == "encode") {
      return new FileEncoder();
    }
    if (mode == "identity") {
      return new IdentityFileProcessor();
    }
    if (mode == "compress") {
      return new FileCompressor("Hello", "World");
    }
    if (mode == "encrypt") {
      return new FileEncryptor();
    }
  }

private:
  FileProcessorFactorySingleton() = default;
  static FileProcessorFactorySingleton *instance;
};

FileProcessorFactorySingleton *FileProcessorFactorySingleton::instance =
    nullptr;

int main(int argc, char *argv[])

{
  const std::string mode = argv[1];
  FileProcessor *fp =
      FileProcessorFactorySingleton::getInstance().createFileProcessor(mode);
  try {
    std::vector<std::string> filesToProcess;
    for (int i = 2; i < argc; ++i) {
      filesToProcess.push_back(argv[i]);
    }

    for (auto file : filesToProcess) {
      fp->processFile(file);
    }

    if (fp) {
      delete fp;
    }
    return 1;

  } catch (...) {
    if (fp) {
      delete fp;
    }
    std::cout << "Sometihing wrong";
    return 0;
  }
}
