#include "xwift/Frontend/Frontend.h"
#include "xwift/Basic/Diagnostic.h"
#include "xwift/Sema/Sema.h"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <filesystem>
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include "xwift/Basic/Version.h"
#include "xwift/xwift.h"
#include "xwift/Lexer/Lexer.h"
#include "xwift/Parser/Parser.h"
#include "xwift/Parser/SyntaxParser.h"
#include "xwift/Interpreter/Interpreter.h"

namespace fs = std::filesystem;

namespace xwift {

void setConsoleEncoding() {
#ifdef _WIN32
  SetConsoleOutputCP(CP_UTF8);
  SetConsoleCP(CP_UTF8);
#endif
}

class CompilerInstance {
public:
  int run(std::vector<std::string> &args) {
    if (args.empty()) {
      printHelp();
      return 0;
    }
    
    std::string action = args[0];
    
    if (action == "--version" || action == "-v") {
      printVersion();
      return 0;
    } else if (action == "--help" || action == "-h") {
      printHelp();
      return 0;
    } else if (action == "--test-lexer") {
      std::string source = "func hello() -> Int { return 42 }";
      if (args.size() > 1) {
        source = args[1];
      }
      testLexer(source);
      return 0;
    } else if (action == "run") {
      if (args.size() < 2) {
        std::cout << "error: please specify a file to run" << std::endl;
        return 1;
      }
      return runFile(args[1]);
    } else if (action == "--check") {
      if (args.size() < 2) {
        std::cout << "error: please specify a file to check" << std::endl;
        return 1;
      }
      return checkFile(args[1]);
    } else {
      if (action.find(".xw") != std::string::npos) {
        return runFile(action);
      }
      std::cout << "XWift compiler ready\n";
      std::cout << "Version: " << version::getXWiftVersion() << "\n";
      std::cout << "Input file: " << action << "\n";
    }
    
    return 0;
  }
  
  int runFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cout << "error: cannot open file '" << filename << "'" << std::endl;
      return 1;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    
    try {
      Lexer lexer(source);
      SyntaxParser parser(lexer);
      auto program = parser.parseProgram();
      
      DiagnosticEngine diag;
      diag.setFilename(filename);
      Sema sema(diag);
      sema.setFilename(filename);
      
      if (!sema.visit(program.get())) {
        return 1;
      }
      
      if (diag.hasErrors()) {
        return 1;
      }
      
      Interpreter interpreter;
      
      fs::path filePath(filename);
      std::string basePath = filePath.parent_path().string();
      if (basePath.empty()) {
        basePath = ".";
      }
      interpreter.run(program.get(), basePath);
      
      return 0;
    } catch (const DiagnosticError& e) {
      std::string normPath = filename;
      std::replace(normPath.begin(), normPath.end(), '\\', '/');
      std::cout << normPath << ":" << e.Line << ":" << e.Column << ": "
                << "error: " << e.Message << std::endl;
      return 1;
    } catch (const std::exception& e) {
      std::cout << filename << ":1:1: error: " << e.what() << std::endl;
      return 1;
    }
  }
  
  int checkFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cout << "error: cannot open file '" << filename << "'" << std::endl;
      return 1;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string source = buffer.str();
    
    std::cout << "Checking " << filename << "..." << std::endl;
    
    try {
      Lexer lexer(source);
      SyntaxParser parser(lexer);
      auto program = parser.parseProgram();
      
      DiagnosticEngine diag;
      diag.setFilename(filename);
      Sema sema(diag);
      sema.setFilename(filename);
      
      if (!sema.visit(program.get())) {
        std::cout << "Check failed: semantic errors found" << std::endl;
        return 1;
      }
      
      if (diag.hasErrors()) {
        std::cout << "Check failed: errors found" << std::endl;
        return 1;
      }
      
      std::cout << "Check successful: no errors found" << std::endl;
      
      return 0;
    } catch (const DiagnosticError& e) {
      std::string normPath = filename;
      std::replace(normPath.begin(), normPath.end(), '\\', '/');
      std::cout << normPath << ":" << e.Line << ":" << e.Column << ": "
                << "error: " << e.Message << std::endl;
      return 1;
    } catch (const std::exception& e) {
      std::cout << filename << ":1:1: error: " << e.what() << std::endl;
      return 1;
    }
  }
  
  void printVersion() {
    std::cout << "xwift version " << version::getXWiftVersion() << "\n";
#ifdef _WIN32
    std::cout << "Target: x86_64-pc-windows-msvc\n";
    std::cout << "Thread model: win32\n";
#elif __linux__
    std::cout << "Target: x86_64-unknown-linux-gnu\n";
    std::cout << "Thread model: posix\n";
#elif __APPLE__
    std::cout << "Target: x86_64-apple-darwin\n";
    std::cout << "Thread model: posix\n";
#else
    std::cout << "Target: unknown\n";
    std::cout << "Thread model: unknown\n";
#endif
  }
  
  void printHelp() {
    std::cout << "XWift Compiler\n";
    std::cout << "Usage: xwift [options] <input files>\n";
    std::cout << "\nOptions:\n";
    std::cout << "  -v, --version   Print version information\n";
    std::cout << "  -h, --help      Display available options\n";
    std::cout << "  --test-lexer    Test lexer with source code\n";
    std::cout << "  run <file>      Run a .xw source file\n";
    std::cout << "  --check <file>  Check a .xw source file for errors\n";
    std::cout << "\nExamples:\n";
    std::cout << "  xwift hello.xw       Run hello.xw\n";
    std::cout << "  xwift run hello.xw   Run hello.xw\n";
    std::cout << "  xwift --check hello.xw  Check hello.xw for errors\n";
  }
};

}

int main(int argc, char **argv) {
  xwift::setConsoleEncoding();
  setvbuf(stdout, NULL, _IONBF, 0);
  
  std::vector<std::string> args;
  for (int i = 1; i < argc; ++i) {
    args.push_back(argv[i]);
  }
  
  xwift::CompilerInstance instance;
  return instance.run(args);
}
