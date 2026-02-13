const vscode = require('vscode');
const fs = require('fs');
const path = require('path');

const keywords = [
    'func', 'var', 'let', 'if', 'else', 'for', 'while', 'switch', 'case', 'default',
    'return', 'break', 'continue', 'import', 'class', 'struct', 'enum', 'protocol',
    'extension', 'guard', 'defer', 'do', 'try', 'catch', 'throw', 'throws',
    'rethrows', 'async', 'await', 'static', 'private', 'public', 'internal',
    'fileprivate', 'open', 'final', 'override', 'mutating', 'lazy', 'weak',
    'unowned', 'init', 'deinit', 'get', 'set', 'willSet', 'didSet',
    'subscript', 'typealias', 'associatedtype', 'where', 'inout', 'some', 'any',
    'is', 'as', 'nil', 'true', 'false', 'in'
];

const types = [
    'Int', 'Int8', 'Int16', 'Int32', 'Int64', 'UInt', 'UInt8', 'UInt16', 'UInt32', 'UInt64',
    'Float', 'Float32', 'Float64', 'Double', 'String', 'Character', 'Bool',
    'Array', 'Dictionary', 'Set', 'Optional', 'Any', 'AnyObject', 'Void', 'Never',
    'Data', 'Date', 'URL', 'UUID'
];

const builtinFunctions = [
    { name: 'print', detail: 'Print to console', documentation: 'Prints values to console without newline' },
    { name: 'println', detail: 'Print to console', documentation: 'Prints values to console with newline' },
    { name: 'read', detail: 'Read input', documentation: 'Reads a line from standard input as string' },
    { name: 'readInt', detail: 'Read integer', documentation: 'Reads a line from standard input as integer' },
    { name: 'len', detail: 'Get length', documentation: 'Returns length of a string' },
    { name: 'toString', detail: 'Convert to string', documentation: 'Converts a value to string' },
    { name: 'toInt', detail: 'Convert to integer', documentation: 'Converts a value to integer' }
];

let diagnosticCollection;

function activate(context) {
    diagnosticCollection = vscode.languages.createDiagnosticCollection('xwift');

    const completionProvider = vscode.languages.registerCompletionItemProvider('xwift', {
        provideCompletionItems(document, position, token, context) {
            const linePrefix = getLinePrefix(document, position);
            const items = [];

            if (linePrefix.length === 0) {
                return items;
            }

            for (const keyword of keywords) {
                if (keyword.startsWith(linePrefix)) {
                    const item = new vscode.CompletionItem(keyword, vscode.CompletionItemKind.Keyword);
                    item.detail = 'keyword';
                    item.insertText = keyword;
                    items.push(item);
                }
            }

            for (const type of types) {
                if (type.startsWith(linePrefix)) {
                    const item = new vscode.CompletionItem(type, vscode.CompletionItemKind.Class);
                    item.detail = 'type';
                    item.insertText = type;
                    items.push(item);
                }
            }

            for (const func of builtinFunctions) {
                if (func.name.startsWith(linePrefix)) {
                    const item = new vscode.CompletionItem(func.name, vscode.CompletionItemKind.Function);
                    item.detail = func.detail;
                    item.documentation = func.documentation;
                    item.insertText = func.name + '()';
                    items.push(item);
                }
            }

            return items;
        }
    });

    context.subscriptions.push(completionProvider);
    context.subscriptions.push(diagnosticCollection);

    const documentSelector = { scheme: 'file', language: 'xwift' };
    const changeDisposable = vscode.workspace.onDidChangeTextDocument(event => {
        if (event.document.languageId === 'xwift') {
            validateDocument(event.document);
        }
    });
    context.subscriptions.push(changeDisposable);

    const openDisposable = vscode.workspace.onDidOpenTextDocument(document => {
        if (document.languageId === 'xwift') {
            validateDocument(document);
        }
    });
    context.subscriptions.push(openDisposable);
}

function validateDocument(document) {
    const diagnostics = [];
    const text = document.getText();

    const bracketErrors = checkBrackets(text);
    for (const error of bracketErrors) {
        const diagnostic = createDiagnostic(
            error.message,
            error.line,
            error.column,
            error.column + 1,
            vscode.DiagnosticSeverity.Error,
            error.code
        );
        diagnostics.push(diagnostic);
    }

    const stringErrors = checkStrings(text);
    for (const error of stringErrors) {
        const diagnostic = createDiagnostic(
            error.message,
            error.line,
            error.column,
            error.column + 1,
            vscode.DiagnosticSeverity.Error,
            error.code
        );
        diagnostics.push(diagnostic);
    }

    const undefinedErrors = checkUndefinedVariables(text, document.uri);
    for (const error of undefinedErrors) {
        const diagnostic = createDiagnostic(
            error.message,
            error.line,
            error.column,
            error.column + error.length,
            vscode.DiagnosticSeverity.Error,
            error.code
        );
        diagnostics.push(diagnostic);
    }

    const duplicateErrors = checkDuplicateDeclarations(text);
    for (const error of duplicateErrors) {
        const diagnostic = createDiagnostic(
            error.message,
            error.line,
            error.column,
            error.column + error.length,
            vscode.DiagnosticSeverity.Warning,
            error.code
        );
        diagnostics.push(diagnostic);
    }

    diagnosticCollection.set(document.uri, diagnostics);
}

function getImportedFunctions(moduleName, documentUri) {
    const importedFunctions = new Set();
    
    const documentDir = path.dirname(documentUri.fsPath);
    const modulePath = path.join(documentDir, moduleName + '.xw');
    
    if (fs.existsSync(modulePath)) {
        const moduleContent = fs.readFileSync(modulePath, 'utf-8');
        const funcMatches = moduleContent.matchAll(/\bfunc\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\(/g);
        if (funcMatches) {
            for (const match of funcMatches) {
                importedFunctions.add(match[1]);
            }
        }
    }
    
    return importedFunctions;
}

function checkBrackets(text) {
    const errors = [];
    const lines = text.split('\n');
    const stack = [];
    const pairs = { '{': '}', '[': ']', '(': ')' };

    for (let lineNum = 0; lineNum < lines.length; lineNum++) {
        const line = lines[lineNum];
        for (let col = 0; col < line.length; col++) {
            const char = line[col];
            if (char === '{' || char === '[' || char === '(') {
                stack.push({ char, line: lineNum + 1, col });
            } else if (char === '}' || char === ']' || char === ')') {
                if (stack.length === 0) {
                    errors.push({ 
                        message: `Unexpected closing bracket '${char}'`, 
                        line: lineNum + 1, 
                        column: col,
                        code: 'XW1002'
                    });
                } else {
                    const last = stack.pop();
                    if (pairs[last.char] !== char) {
                        errors.push({ 
                            message: `Mismatched brackets: expected '${pairs[last.char]}' but found '${char}'`, 
                            line: lineNum + 1, 
                            column: col,
                            code: 'XW1003'
                        });
                    }
                }
            }
        }
    }

    for (const unclosed of stack) {
        errors.push({ 
            message: `Unclosed bracket '${unclosed.char}'`, 
            line: unclosed.line, 
            column: unclosed.col,
            code: 'XW1004'
        });
    }

    return errors;
}

function checkStrings(text) {
    const errors = [];
    const lines = text.split('\n');

    for (let lineNum = 0; lineNum < lines.length; lineNum++) {
        const line = lines[lineNum];
        let inString = false;
        let stringStart = 0;
        let quoteCount = 0;

        for (let i = 0; i < line.length; i++) {
            const char = line[i];
            if (char === '"' && (i === 0 || line[i - 1] !== '\\')) {
                quoteCount++;
                if (!inString) {
                    inString = true;
                    stringStart = i;
                } else {
                    inString = false;
                }
            }
        }

        if (quoteCount % 2 !== 0) {
            errors.push({ 
                message: 'Unterminated string literal', 
                line: lineNum + 1,
                column: stringStart,
                code: 'XW1001'
            });
        }
    }

    return errors;
}

function checkUndefinedVariables(text, documentUri) {
    const errors = [];
    const lines = text.split('\n');
    const definedVariables = new Set();
    const definedFunctions = new Set();
    const allParameters = new Set();
    const importedModules = new Set();
    const importedFunctions = new Set();
    const forLoopVariables = new Set();

    for (let lineNum = 0; lineNum < lines.length; lineNum++) {
        const line = lines[lineNum];

        const importMatch = line.match(/\bimport\s+([a-zA-Z_][a-zA-Z0-9_]*)/);
        if (importMatch) {
            const moduleName = importMatch[1];
            importedModules.add(moduleName);
            const moduleFunctions = getImportedFunctions(moduleName, documentUri);
            moduleFunctions.forEach(func => importedFunctions.add(func));
            continue;
        }

        const forMatch = line.match(/\bfor\s+([a-zA-Z_][a-zA-Z0-9_]*)\s+in\s+/);
        if (forMatch) {
            const loopVar = forMatch[1];
            forLoopVariables.add(loopVar);
        }

        const funcMatch = line.match(/\bfunc\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\(([^)]*)\)/);
        if (funcMatch) {
            const funcName = funcMatch[1];
            definedFunctions.add(funcName);
            
            const paramsStr = funcMatch[2];
            const paramMatches = paramsStr.matchAll(/([a-zA-Z_][a-zA-Z0-9_]*)\s*:/g);
            if (paramMatches) {
                for (const match of paramMatches) {
                    const paramName = match[1];
                    allParameters.add(paramName);
                }
            }
        }

        const varMatch = line.match(/\b(var|let)\s+([a-zA-Z_][a-zA-Z0-9_]*)/);
        if (varMatch) {
            const varName = varMatch[2];
            if (definedVariables.has(varName)) {
                errors.push({
                    message: `Duplicate declaration of '${varName}'`,
                    line: lineNum + 1,
                    column: varMatch.index,
                    length: varMatch[0].length,
                    code: 'XW2001'
                });
            } else {
                definedVariables.add(varName);
            }
        }
    }

    for (let lineNum = 0; lineNum < lines.length; lineNum++) {
        const line = lines[lineNum];

        const usageMatches = line.matchAll(/\b([a-zA-Z_][a-zA-Z0-9_]*)\b/g);
        if (usageMatches) {
            for (const match of usageMatches) {
                const varName = match[1];
                
                const isKeyword = keywords.includes(varName);
                const isType = types.includes(varName);
                const isBuiltin = builtinFunctions.some(f => f.name === varName);
                const isFunction = definedFunctions.has(varName);
                const isDefined = definedVariables.has(varName);
                const isParameter = allParameters.has(varName);
                const isForLoopVar = forLoopVariables.has(varName);
                const isImportedModule = importedModules.has(varName);
                const isImportedFunction = importedFunctions.has(varName);
                
                const isInsideString = isInsideStringLiteral(line, match.index);
                const isAfterFunc = line.substring(0, match.index).match(/\bfunc\s+$/);
                const isAfterArrow = line.substring(0, match.index).match(/->\s*$/);
                const isAfterVarLet = line.substring(0, match.index).match(/\b(var|let)\s+$/);
                const isAfterColon = line.substring(0, match.index).match(/:\s*$/);
                const isAfterImport = line.substring(0, match.index).match(/\bimport\s+$/);
                const isAfterFor = line.substring(0, match.index).match(/\bfor\s+$/);
                const isAfterIn = line.substring(0, match.index).match(/\bin\s+$/);
                
                if (!isKeyword && !isType && !isBuiltin && !isFunction && !isDefined && !isParameter && !isForLoopVar && !isInsideString && !isAfterFunc && !isAfterArrow && !isAfterVarLet && !isAfterColon && !isAfterImport && !isImportedModule && !isAfterFor && !isAfterIn && !isImportedFunction) {
                    errors.push({
                        message: `Use of undefined variable '${varName}'`,
                        line: lineNum + 1,
                        column: match.index,
                        length: varName.length,
                        code: 'XW2002'
                    });
                }
            }
        }
    }

    return errors;
}

function isInsideStringLiteral(line, position) {
    let inString = false;
    for (let i = 0; i < position; i++) {
        if (line[i] === '"' && (i === 0 || line[i - 1] !== '\\')) {
            inString = !inString;
        }
    }
    return inString;
}

function checkDuplicateDeclarations(text) {
    const errors = [];
    const lines = text.split('\n');
    const declaredFunctions = new Map();

    for (let lineNum = 0; lineNum < lines.length; lineNum++) {
        const line = lines[lineNum];

        const funcMatch = line.match(/\bfunc\s+([a-zA-Z_][a-zA-Z0-9_]*)\s*\(/);
        if (funcMatch) {
            const funcName = funcMatch[1];
            if (declaredFunctions.has(funcName)) {
                const prevDecl = declaredFunctions.get(funcName);
                errors.push({
                    message: `Duplicate function declaration '${funcName}' (previously declared at line ${prevDecl.line})`,
                    line: lineNum + 1,
                    column: funcMatch.index,
                    length: funcMatch[0].length,
                    code: 'XW2003'
                });
            } else {
                declaredFunctions.set(funcName, { line: lineNum + 1 });
            }
        }
    }

    return errors;
}

function createDiagnostic(message, line, start, end, severity, code) {
    const range = new vscode.Range(
        new vscode.Position(line - 1, start),
        new vscode.Position(line - 1, end)
    );
    const diagnostic = new vscode.Diagnostic(range, message, severity);
    diagnostic.source = 'xwift';
    diagnostic.code = code;
    return diagnostic;
}

function getLinePrefix(document, position) {
    const range = new vscode.Range(
        new vscode.Position(position.line, 0),
        position
    );
    const text = document.getText(range);
    const match = text.match(/[\w.]*$/);
    return match ? match[0] : '';
}

function deactivate() {
    if (diagnosticCollection) {
        diagnosticCollection.dispose();
    }
}

module.exports = {
    activate,
    deactivate
};
