"use client"

import { useState, useRef, useCallback, useEffect } from "react" 

const API_URL = "http://localhost:3001"

// -------------------- Utilities --------------------
const runLexicalAnalysis = async (code) => {
  const response = await fetch(`${API_URL}/lexical`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ code }),
  })
  if (!response.ok) {
    const error = await response.json()
    throw new Error(error.error || "Analysis failed")
  }
  return response.json()
}

// -------------------- Home Page Component --------------------
const HomePage = ({ setView, darkMode }) => {
  const btnBase = {
    padding: "16px 32px",
    fontSize: "14px",
    fontWeight: "700",
    borderRadius: "8px",
    cursor: "pointer",
    transition: "all 0.2s ease",
    textTransform: "uppercase",
    width: "220px",
    border: "none",
    boxShadow: "0 4px 15px rgba(0,0,0,0.3)"
  };

  return (
    <div style={{ 
      height: "100vh", 
      width: "100vw", 
      position: "relative", 
      overflow: "hidden",
      display: "flex",
      justifyContent: "center",
      alignItems: "center",
      background: darkMode ? "#1f2730" : "#6f38e5" 
    }}>
      <img
        src="/HomeBg.gif"
        alt="Background"
        style={{
          position: "absolute",
          top: 0,
          left: 0,
          width: "100%",
          height: "100%",
          objectFit: "cover",
          zIndex: 0,
          filter: "brightness(0.8)",
          pointerEvents: "none"
        }}
      />

      {/* Content Overlay - Added transform to nudge content up for visual centering */}
      <div style={{ 
        zIndex: 10, 
        textAlign: "center", 
        color: "#ffffff", 
        position: "relative",
        transform: "translateY(-40px)" // Moves contents up slightly
      }}>
        <img src="Cnack_nobg.png" alt="Logo" style={{ height: "120px", marginBottom: "20px" }} />
        <h1 style={{ fontSize: "48px", margin: "0 0 10px 0", textShadow: "0 2px 10px rgba(0,0,0,0.5)" }}>
          Cnack Mini Compiler
        </h1>
        <p style={{ fontSize: "18px", opacity: 0.9, marginBottom: "40px", fontWeight: "500" }}>
          An environment for lexical and syntax analysis of Cnack language.
        </p>
        
        <div style={{ display: "flex", gap: "20px", justifyContent: "center" }}>
          <button 
            onClick={() => setView("lexical")}
            style={{ ...btnBase, background: "#ffffff", color: "#0f4687" }}
            onMouseEnter={(e) => e.target.style.transform = "translateY(-3px)"}
            onMouseLeave={(e) => e.target.style.transform = "translateY(0)"}
          >
            Lexical Analyzer
          </button>
          <button 
            style={{ 
              ...btnBase, 
              background: "rgba(255,255,255,0.1)", 
              color: "#ffffff", 
              border: "2px solid #ffffff", 
              opacity: 0.6, 
              cursor: "not-allowed" 
            }}
          >
            Syntax Analyzer
          </button>
        </div>
      </div>
    </div>
  );
};

// -------------------- Code Editor Component --------------------
const CodeEditor = ({ value, onChange, disabled, darkMode }) => {
  const editorRef = useRef(null);
  const lineNumbersRef = useRef(null);
  const highlightRef = useRef(null);

  const handleScroll = useCallback(() => {
    if (editorRef.current && lineNumbersRef.current && highlightRef.current) {
      const { scrollTop, scrollLeft } = editorRef.current;
      lineNumbersRef.current.scrollTop = scrollTop;
      highlightRef.current.scrollTop = scrollTop;
      highlightRef.current.scrollLeft = scrollLeft;
    }
  }, []);

  useEffect(() => {
    handleScroll();
  }, [value, handleScroll]);

  const handleKeyDown = (e) => {
    if (disabled) return;
    const start = e.target.selectionStart;
    const end = e.target.selectionEnd;

    if (e.key === "Tab") {
      e.preventDefault();
      const newValue = value.substring(0, start) + "    " + value.substring(end);
      onChange(newValue);
      setTimeout(() => {
        editorRef.current.selectionStart = editorRef.current.selectionEnd = start + 4;
      }, 0);
    }

    if (e.key === "Enter") {
      e.preventDefault();
      const lines = value.substring(0, start).split("\n");
      const currentLine = lines[lines.length - 1];
      const match = currentLine.match(/^(\s*)/);
      let indent = match ? match[0] : "";
      
      const trimmedLine = currentLine.trim();
      if (trimmedLine.endsWith("{") || trimmedLine.endsWith("(") || trimmedLine.endsWith("[")) {
        indent += "    ";
      }


      const newValue = value.substring(0, start) + "\n" + indent + value.substring(end);
      onChange(newValue);
      setTimeout(() => {
        editorRef.current.selectionStart = editorRef.current.selectionEnd = start + 1 + indent.length;
      }, 0);
    }
  };

  const renderHighlightedCode = () => {
  const words = value.split(/(\/\/[^\n]*|\/\*[\s\S]*?\*\/|"(?:\\.|[^\\"])*"|\s+|<<|>>|==|!=|<=|>=|&&|\|\||\*\||[()\[\]{};,:]|[-+*/%<>&!|=])/);
  
  return words.map((word, i) => {
    if (!word) return null;
    
    let color = darkMode ? "#e2e8f0" : "#0f4687"; 

    if (word.startsWith('"') && word.endsWith('"')) {
      const stringColor = "#80a6c0ff";
      const parts = word.split(/(\{.*?\})/g);
      
      return (
        <span key={i} style={{ color: stringColor }}>
          {parts.map((part, j) => {
            if (part.startsWith('{') && part.endsWith('}')) {
              const content = part.slice(1, -1);
              const subTokens = content.split(/(\*|\||[-+/%<>&!=])/g);
              
              return (
                <span key={j}>
                  <span style={{ color: darkMode ? "#94a3b8" : "#4a89c6" }}>{'{'}</span>
                  {subTokens.map((st, k) => {
                    let stColor = darkMode ? "#e2e8f0" : "#0f4687";
                    // Color symbols/operators inside the braces (like the * in *scorePtr)
                    if (/^([-+*/%<>&!|=])$/.test(st)) {
                      stColor = "#b63388ff"; 
                    }
                    return <span key={k} style={{ color: stColor }}>{st}</span>;
                  })}
                  <span style={{ color: darkMode ? "#94a3b8" : "#4a89c6" }}>{'}'}</span>
                </span>
              );
            }
            return part;
          })}
        </span>
      );
    }

    // Comments
    if (word.startsWith("//") || word.startsWith("/*")) {
      color = "#6272a4"; 
    } 
    // Keywords
    else if (/^(int|string|float|char|double|boolean|void|if|else|elif|switch|case|default|assign|otherwise|while|for|do while|break|continue|return|execute|exit|const|ask|display|true|false|fetch|when)$/.test(word)) {
      color = "#437ae6ff"; 
    } 
    // Numbers
    else if (!isNaN(word) && word.trim() !== "") {
      color = "#bd93f9"; 
    } 
    // Symbols
    else if (/^(\(|\)|\{|\}|\[|\]|;|,|:)$/.test(word)) {
      color = darkMode ? "#6290d0ff" : "#4a89c6"; 
    } 
    // Operators 
    else if (/^(=|\+|-|\*|\/|%|<|>|&|!|<<|>>|==|!=|<=|>=|&&|\|\||\*\|)$/.test(word)) {
      color = "#b63388ff"; 
    }

    return <span key={i} style={{ color }}>{word}</span>;
  });
};

  const sharedStyles = {
    fontFamily: '"Fira Code", "Consolas", monospace',
    fontSize: "13px",
    lineHeight: "21px", 
    paddingTop: "20px",
    paddingBottom: "20px",
    tabSize: "4",
    MozTabSize: "4",
    whiteSpace: "pre",
    wordBreak: "keep-all",
    boxSizing: "border-box",
  };

  return (
    <div style={{ display: "flex", height: "100%", background: darkMode ? "#1f2730" : "#ffffff", overflow: "hidden" }}>
      {/* Line Numbers */}
      <div
        ref={lineNumbersRef}
        style={{
          ...sharedStyles,
          width: "70px",
          background: darkMode ? "#28313b" : "#f8fafc",
          borderRight: `1px solid ${darkMode ? "#334155" : "#e0e7ff"}`,
          color: darkMode ? "#64748b" : "#4a89c6",
          textAlign: "right",
          paddingRight: "16px",
          overflow: "hidden", 
          userSelect: "none",
        }}
      >
        <pre style={{ margin: 0, ...sharedStyles, paddingTop: 0 }}>
          {value.split("\n").map((_, i) => i + 1).join("\n")}
        </pre>
      </div>

      <div style={{ flex: 1, position: "relative", overflow: "hidden" }}>
        {/* Highlight Layer */}
        <div
          ref={highlightRef}
          style={{
            ...sharedStyles,
            position: "absolute",
            top: 0,
            left: 0,
            right: 0,
            bottom: 0,
            paddingLeft: "24px",
            paddingRight: "20px",
            paddingBottom: "61px",
            pointerEvents: "none",
            overflow: "hidden",
            color: "transparent",
            backgroundColor: "transparent",
          }}
        >
          {renderHighlightedCode()}
        </div>

        {/* Input Layer */}
        <textarea
          ref={editorRef}
          value={value}
          onChange={(e) => onChange(e.target.value)}
          onKeyDown={handleKeyDown}
          onScroll={handleScroll}
          disabled={disabled}
          spellCheck={false}
          style={{
            ...sharedStyles,
            width: "100%",
            height: "100%",
            paddingLeft: "24px",
            background: "transparent",
            color: "transparent", 
            caretColor: darkMode ? "#ffffff" : "#0f4687", 
            resize: "none",
            border: "none",
            outline: "none",
            overflow: "auto",
            position: "relative",
            zIndex: 1,
          }}
        />
      </div>
    </div>
  );
};

// -------------------- Parsing Output --------------------
const parseTokenOutput = (output) => {
  const lines = output.split("\n");
  const tokens = []
  let startParsing = false

  for (const line of lines) {
    if (line.includes("-------|")) {
      startParsing = true
      continue
    }
    if (startParsing && line.trim()) {
      const first = line.indexOf("|")
      const second = line.indexOf("|", first + 1)
      if (first !== -1 && second !== -1) {
        const lineNum = line.slice(0, first).trim()
        const type = line.slice(first + 1, second).trim()
        const lexeme = line.slice(second + 1).trim()
        tokens.push({ line: lineNum, type, lexeme })
      }
    }
  }
  return tokens
}

// -------------------- Output Component --------------------
const Output = ({ output, error, loading, darkMode }) => {
  const [filterLine, setFilterLine] = useState("");

  const parseTokenOutput = (out) => {
    const lines = out.split("\n");
    const tokens = []
    let startParsing = false
    for (const line of lines) {
      if (line.includes("-------|")) { startParsing = true; continue }
      if (startParsing && line.trim()) {
        const first = line.indexOf("|")
        const second = line.indexOf("|", first + 1)
        if (first !== -1 && second !== -1) {
          const lineNum = line.slice(0, first).trim()
          const type = line.slice(first + 1, second).trim()
          const lexeme = line.slice(second + 1).trim()
          tokens.push({ line: lineNum, type, lexeme })
        }
      }
    }
    return tokens
  }

  let tokens = output ? parseTokenOutput(output) : []

  if (filterLine.trim() !== "") {
    tokens = tokens.filter(t => t.line === filterLine.trim());
  }

  const handleDownload = () => {
    if (!output) return
    let content = output
    if (!output.includes("END OF ANALYSIS")) {
      content = output + "\n================================================\n     END OF ANALYSIS\n================================================\n"
    }
    const blob = new Blob([content], { type: 'text/plain' })
    const url = URL.createObjectURL(blob)
    const link = document.createElement('a')
    link.href = url
    link.download = 'lexical_analysis_output.txt'
    document.body.appendChild(link)
    link.click()
    document.body.removeChild(link)
    URL.revokeObjectURL(url)
  }

  return (
    <div style={{ display: "flex", flexDirection: "column", height: "100%", background: darkMode ? "#1f2730" : "#ffffff" }}>
      <div style={{
        height: "60px", padding: "0 24px", background: darkMode ? "#0f4687" : "rgba(15, 69, 135, 1)",
        borderBottom: `1px solid ${darkMode ? "#334155" : "#c7d2fe"}`,
        color: "#ffffff", fontWeight: "600", fontSize: "13px", textTransform: "uppercase",
        display: "flex", justifyContent: "space-between", alignItems: "center",
      }}>
        <div style={{ display: "flex", alignItems: "center", gap: "16px" }}>
          <span>Lexical Analysis</span>
          
          {/* LINE FILTER INPUT */}
          <div style={{ 
            display: "flex", 
            alignItems: "center", 
            background: darkMode ? "#275893" : "rgba(255,255,255,0.1)", 
            borderRadius: "4px", 
            padding: "2px 10px", 
            border: darkMode ? "1px solid #4a89c6" : "1px solid rgba(255,255,255,0.2)",
            marginBottom: "-1px",
            height: "16px" 
          }}>
            <span style={{ 
              fontSize: "10px", 
              color: "#ffffff", 
              marginRight: "4px",
              display: "flex",
              alignItems: "center"
            }}>LINE:</span>
            
            <div style={{ position: "relative", display: "flex", alignItems: "center" }}>
              
              {!filterLine && (
                <span style={{
                  position: "absolute",
                  left: "0",
                  width: "100%",
                  textAlign: "center",
                  color: "#ffffff",
                  fontSize: "10px",
                  pointerEvents: "none",
                  opacity: 0.9
                }}>
                  All
                </span>
              )}
              <input 
                type="text" 
                value={filterLine}
                onChange={(e) => setFilterLine(e.target.value)}
                style={{
                  width: "40px",
                  background: "transparent",
                  border: "none",
                  outline: "none",
                  color: "#ffffff",
                  fontSize: "12px",
                  fontWeight: "600",
                  textAlign: "center",
                  lineHeight: "1",
                  padding: 0,
                  margin: 0
                }}
              />
            </div>
          </div>
        </div>

        {output && !loading && (
          <button
            onClick={handleDownload}
            style={{
              padding: "6px 16px", background: "#ffffff", color: "#0f4687", border: "1px solid #c7d2fe",
              borderRadius: "6px", fontSize: "11px", fontWeight: "600", cursor: "pointer",
              transition: "all 0.15s", display: "flex", alignItems: "center", gap: "6px", textTransform: "uppercase",
            }}
            onMouseEnter={(e) => { e.currentTarget.style.background = "#c7d2fe"; e.currentTarget.style.color = "#0e5398" }}
            onMouseLeave={(e) => { e.currentTarget.style.background = "#ffffff"; e.currentTarget.style.color = "#0f4687" }}
          >
            Download
          </button>
        )}
      </div>

      <div style={{ flex: 1, overflowY: "auto", padding: "24px" }}>
        {error ? (
          <div style={{ background: "rgba(248, 81, 73, 0.1)", border: "1px solid rgba(248, 81, 73, 0.3)", padding: "16px", borderRadius: "8px", color: "#d32f2f", fontSize: "13px" }}>
            <strong>Error</strong>: {error}
          </div>
        ) : loading ? (
          <div style={{ textAlign: "center", padding: "40px", color: "#4a89c6" }}>Analyzing...</div>
        ) : tokens.length > 0 ? (
          <div style={{ background: darkMode ? "#28313b" : "#f8fafc", borderRadius: "8px", overflow: "hidden", border: `1px solid ${darkMode ? "#334155" : "#e0e7ff"}` }}>
            <div style={{ display: "grid", gridTemplateColumns: "repeat(3, 1fr)", padding: "16px 20px", background: "#4a89c6", fontWeight: "600", fontSize: "12px", color: "#ffffff", textAlign: "center" }}>
              <div>Line</div><div>Type</div><div>Lexeme</div>
            </div>
            {tokens.map((t, i) => (
              <div key={i} style={{ 
                display: "grid", gridTemplateColumns: "repeat(3, 1fr)", padding: "12px 20px", 
                background: i % 2 === 0 ? (darkMode ? "#1f2730" : "#ffffff") : (darkMode ? "#28313b" : "#f8fafc"), 
                borderBottom: `1px solid ${darkMode ? "#334155" : "#e0e7ff"}`, 
                fontSize: "13px", textAlign: "center", color: darkMode ? "#cbd5e1" : "#0f4687" 
              }}>
                <div>{t.line}</div><div>{t.type}</div><div>{t.lexeme}</div>
              </div>
            ))}
          </div>
        ) : (
          <div style={{ textAlign: "center", padding: "40px", color: "#4a89c6", fontSize: "13px" }}>
            {filterLine ? `No tokens found for line ${filterLine}.` : "No analysis yet."}
          </div>
        )}
      </div>
    </div>
  )
}

// -------------------- Editor Component --------------------
const Editor = ({ code, setCode, onRun, onClear, loading, darkMode }) => {
  return (
    <div style={{ height: "100%", display: "flex", flexDirection: "column" }}>
      <div
        style={{
          height: "60px",
          padding: "0 24px",
          background: darkMode ? "#0f4687" : "rgba(15, 69, 135, 1)",
          borderBottom: `1px solid ${darkMode ? "#334155" : "#e0e7ff"}`,
          display: "flex",
          justifyContent: "space-between",
          alignItems: "center",
        }}
      >
        <div style={{ color: "#ffffff", fontWeight: "600", fontSize: "13px", textTransform: "uppercase" }}>Code Input</div>
        <button
          onClick={onClear}
          disabled={loading}
          style={{
            padding: "6px 16px",
            background: "#ffffff",
            borderRadius: "6px",
            color: "#0f4687",
            border: "1px solid #c7d2fe",
            cursor: loading ? "not-allowed" : "pointer",
            fontSize: "11px",
            fontWeight: "500",
            transition: "all 0.15s",
            textTransform: "uppercase"
          }}
          onMouseEnter={(e) => { if (!loading) { e.target.style.background = "#c7d2fe"; e.target.style.color = "#0e5398" } }}
          onMouseLeave={(e) => { if (!loading) { e.target.style.background = "#ffffff"; e.target.style.color = "#0f4687" } }}
        >
          CLEAR
        </button>
      </div>

      <div style={{ flex: 1, overflow: "hidden" }}>
        <CodeEditor value={code} onChange={setCode} disabled={loading} darkMode={darkMode} />
      </div>

      <div style={{ padding: "16px 24px", borderTop: `1px solid ${darkMode ? "#334155" : "#e0e7ff"}`, background: darkMode ? "#1f2730" : "#ffffff" }}>
        <button
          onClick={onRun}
          disabled={loading}
          style={{
            width: "100%",
            padding: "12px",
            background: loading ? "#4a89c6" : "#0f4687",
            color: "#ffffff",
            border: "none",
            borderRadius: "6px",
            fontWeight: "600",
            fontSize: "13px",
            cursor: loading ? "not-allowed" : "pointer",
          }}
        >
          {loading ? "Analyzing..." : "Analyze Code"}
        </button>
      </div>
    </div>
  )
}

// -------------------- Main App --------------------
const App = () => {
  const [view, setView] = useState("home"); 
  const [darkMode, setDarkMode] = useState(() => {
    const saved = localStorage.getItem("theme")
    return saved ? saved === "dark" : window.matchMedia("(prefers-color-scheme: dark)").matches
  })

  useEffect(() => {
    localStorage.setItem("theme", darkMode ? "dark" : "light")
  }, [darkMode])

  const [code, setCode] = useState("// Cnack Mini Compiler\n execute() {\n    int x = 10;\n    exit();\n}")
  const [output, setOutput] = useState("")
  const [error, setError] = useState("")
  const [loading, setLoading] = useState(false)

  const handleRun = async () => {
    if (!code.trim()) return setError("Please enter code to analyze.")
    setLoading(true); setError(""); setOutput("")
    try {
      const result = await runLexicalAnalysis(code)
      setOutput(result.output)
    } catch (err) { setError(err.message) }
    finally { setLoading(false) }
  }

  if (view === "home") {
    return <HomePage setView={setView} darkMode={darkMode} />;
  }

  return (
    <div style={{
      display: "flex", flexDirection: "column", width: "100vw", height: "100vh",
      background: darkMode ? "#28313b" : "#d8e6f4ff", transition: "all 0.3s ease"
    }}>
      {/* App Header */}
      <div style={{
        padding: "16px 24px", background: darkMode ? "#1f2730" : "#f8fafc",
        borderBottom: `1px solid ${darkMode ? "#334155" : "#e0e7ff"}`,
        display: "flex", justifyContent: "space-between", alignItems: "center"
      }}>
        <div style={{ display: "flex", alignItems: "center", gap: "8px" }}> {/* Reduced gap */}
          {/* Back Button - Scaled down and padding reduced */}
          <button 
            onClick={() => setView("home")}
            style={{ 
              background: "none", 
              border: "none", 
              cursor: "pointer", 
              fontSize: "16px", // Reduced size
              color: darkMode ? "#fff" : "#0f4687", 
              padding: "4px", // Minimal padding
              display: "flex",
              alignItems: "center",
              justifyContent: "center"
            }}
          >
            ←
          </button>
          <img src="Cnack_nobg.png" alt="Logo" style={{ height: "40px" }} /> {/* Slightly smaller logo */}
          <h1 style={{ margin: 0, fontSize: "18px", color: darkMode ? "#f8fafc" : "#0f4687" }}>Cnack Mini Compiler</h1>
          
          <div 
            onClick={() => setDarkMode(!darkMode)}
            style={{
              marginLeft: "12px", width: "42px", height: "22px", background: darkMode ? "#0f4687" : "#cbd5e1",
              borderRadius: "20px", cursor: "pointer", display: "flex", alignItems: "center", padding: "2px", transition: "0.3s"
            }}
          >
            <div style={{
              width: "18px", height: "18px", background: "#fff", borderRadius: "50%",
              transform: darkMode ? "translateX(20px)" : "translateX(0)", transition: "0.3s",
              display: "flex", alignItems: "center", justifyContent: "center", fontSize: "10px"
            }}>
              {darkMode ? "🌙" : "☀️"}
            </div>
          </div>
        </div>
        <div style={{ fontSize: "12px", color: darkMode ? "#94a3b8" : "#4a89c6" }}>Lexical Analyzer</div>
      </div>

      <div style={{ display: "grid", gridTemplateColumns: "1fr 1fr", flex: 1, gap: "16px", padding: "16px", overflow: "hidden" }}>
        <div style={{ background: darkMode ? "#1f2730" : "#ffffff", borderRadius: "8px", border: `1px solid ${darkMode ? "#334155" : "#b9babdff"}`, overflow: "hidden" }}>
          <Editor code={code} setCode={setCode} onRun={handleRun} onClear={() => setCode("")} loading={loading} darkMode={darkMode} />
        </div>
        <div style={{ background: darkMode ? "#1f2730" : "#ffffff", borderRadius: "8px", border: `1px solid ${darkMode ? "#334155" : "#b9babdff"}`, overflow: "hidden" }}>
          <Output output={output} error={error} loading={loading} darkMode={darkMode} />
        </div>
      </div>
    </div>
  )
}

export default App