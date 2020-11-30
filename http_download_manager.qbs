import qbs

CppApplication {
    consoleApplication: true
    files: [
        "inc/download/agent/base.hpp",
        "inc/download/agent/http.hpp",
        "inc/download/agent/https.hpp",
        "inc/download/manager.hpp",
        "inc/download/task.hpp",
        "main.cpp",
    ]
    cpp.cxxLanguageVersion: "c++20"
    cpp.includePaths: ["inc"]
    cpp.staticLibraries: ["pthread", "ssl", "crypto"]
    Group {
        fileTagsFilter: "application"
        qbs.install: true
        qbs.installDir: "bin"
    }
}
