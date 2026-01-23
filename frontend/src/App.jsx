"use client"

import { useState, useRef, useCallback, useEffect } from "react" 

const API_URL = "http://localhost:3001"

// ===========================
// 1. UTILITIES & API
// ===========================
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

const runSyntaxAnalysis = async (code) => {
  const response = await fetch(`${API_URL}/syntax`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ code }),
  })
  if (!response.ok) {
    const error = await response.json()
    // Return error output if available (e.g. syntax errors from parser)
    if (error.output) return error;
    throw new Error(error.error || "Analysis failed")
  }
  return response.json()
}

// ===========================
// 2. COMPONENTS
// ===========================

// --- HOME PAGE (Landing View) ---
const HomePage = ({ onStart }) => {
  return (
    <div style={{ 
      height: "100vh", width: "100vw", display: "flex", flexDirection: "column", 
      justifyContent: "center", alignItems: "center", position: "relative",
      background: "#020617", overflow: "hidden"
    }}>
      {/* Background Image Layer */}
      <div style={{
        position: "absolute", top: 0, left: 0, width: "100%", height: "100%",
        backgroundImage: "url('/HomeBg.gif')", 
        backgroundSize: "cover",
        backgroundPosition: "center",
        opacity: 0.5,
        zIndex: 0
      }}></div>

      <div style={{ zIndex: 10, textAlign: "center", color: "white", transform: "translateY(-20px)" }}>
        <img src="Cnack_nobg.png" alt="Cnack Logo" style={{ height: "110px", marginBottom: "24px", filter: "drop-shadow(0 0 10px rgba(255,255,255,0.2))" }} />
        <h1 style={{ fontSize: "64px", fontWeight: "800", margin: "0 0 12px 0", letterSpacing: "-1px" }}>
          Cnack Mini Compiler
        </h1>
        <p style={{ fontSize: "18px", color: "#e2e8f0", marginBottom: "48px", fontWeight: "400" }}>
          An environment for lexical and syntax analysis of Cnack language.
        </p>
        
        <button 
          onClick={onStart}
          style={{
            padding: "18px 50px", fontSize: "15px", fontWeight: "800", borderRadius: "6px",
            border: "none", background: "white", color: "#0f172a", cursor: "pointer",
            textTransform: "uppercase", letterSpacing: "1px",
            boxShadow: "0 10px 25px -5px rgba(255, 255, 255, 0.2)",
            transition: "all 0.2s ease"
          }}
          onMouseEnter={(e) => { e.target.style.transform = "translateY(-2px)"; e.target.style.boxShadow = "0 15px 30px -5px rgba(255, 255, 255, 0.4)"; }}
          onMouseLeave={(e) => { e.target.style.transform = "translateY(0)"; e.target.style.boxShadow = "0 10px 25px -5px rgba(255, 255, 255, 0.2)"; }}
        >
          START ANALYZING
        </button>
      </div>
    </div>
  )
}

// --- CODE EDITOR COMPONENT ---
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
  };

  const renderHighlightedCode = () => {
    const words = value.split(/(\/\/[^\n]*|\/\*[\s\S]*?\*\/|"(?:\\.|[^\\"])*"|\s+|<<|>>|==|!=|<=|>=|&&|\|\||\*\||[()\[\]{};,:]|[-+*/%<>&!|=])/);
    
    return words.map((word, i) => {
      if (!word) return null;
      
      let color = darkMode ? "#e2e8f0" : "#0f4687"; 

      if (word.startsWith('"')) {
        return <span key={i} style={{ color: "#80a6c0ff" }}>{word}</span>;
      }
      if (word.startsWith("//") || word.startsWith("/*")) {
        color = "#6272a4"; 
      } 
      else if (/^(int|string|float|char|double|boolean|void|if|else|elif|switch|case|default|assign|otherwise|while|for|do|break|continue|return|execute|exit|const|ask|display|true|false|fetch|when|fn)$/.test(word)) {
        color = "#437ae6ff"; 
      } 
      else if (/^(auto_ref)$/.test(word)) {
        color = "#e6c643";
      }
      else if (!isNaN(word) && word.trim() !== "") {
        color = "#bd93f9"; 
      } 
      else if (/^(\(|\)|\{|\}|\[|\]|;|,|:)$/.test(word)) {
        color = darkMode ? "#6290d0ff" : "#4a89c6"; 
      } 
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
        <div
          ref={highlightRef}
          style={{
            ...sharedStyles,
            position: "absolute",
            top: 0, left: 0, right: 0, bottom: 0,
            paddingLeft: "24px", paddingRight: "20px", paddingBottom: "61px",
            pointerEvents: "none", overflow: "hidden", color: "transparent", backgroundColor: "transparent",
          }}
        >
          {renderHighlightedCode()}
        </div>
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
            width: "100%", height: "100%", paddingLeft: "24px",
            background: "transparent", color: "transparent", 
            caretColor: darkMode ? "#ffffff" : "#0f4687", 
            resize: "none", border: "none", outline: "none",
            overflow: "auto", position: "relative", zIndex: 1,
          }}
        />
      </div>
    </div>
  );
};

// --- OUTPUT COMPONENT ---
const Output = ({ output, error, loading, darkMode, activeTab }) => {
  const [filterLine, setFilterLine] = useState("");

  const parseTokenOutput = (out) => {
    if (!out) return [];
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

  // --- Logic to Render Content based on activeTab ---
  let contentToRender = null;

  if (activeTab === "lexical") {
    // TABLE VIEW
    const tokens = output ? parseTokenOutput(output) : [];
    const filteredTokens = filterLine.trim() !== "" 
        ? tokens.filter(t => t.line === filterLine.trim()) 
        : tokens;

    if (filteredTokens.length > 0) {
        contentToRender = (
            <div style={{ background: darkMode ? "#28313b" : "#f8fafc", borderRadius: "8px", overflow: "hidden", border: `1px solid ${darkMode ? "#334155" : "#e0e7ff"}` }}>
                <div style={{ display: "grid", gridTemplateColumns: "repeat(3, 1fr)", padding: "16px 20px", background: "#4a89c6", fontWeight: "600", fontSize: "12px", color: "#ffffff", textAlign: "center" }}>
                <div>LINE</div><div>TOKEN TYPE</div><div>LEXEME</div>
                </div>
                {filteredTokens.map((t, i) => (
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
        );
    }
  } else {
    // SYNTAX TERMINAL VIEW (Conditional Background)
    if (output) {
        contentToRender = (
            <div style={{ 
                background: darkMode ? "#0f111a" : "#ffffff", // Dark (Terminal Black) vs White
                color: darkMode ? "#e2e8f0" : "#1e293b",      // Light Text vs Dark Text
                padding: "20px", 
                borderRadius: "8px", 
                fontFamily: '"Fira Code", monospace',
                fontSize: "13px",
                lineHeight: "1.6",
                whiteSpace: "pre-wrap",
                border: `1px solid ${darkMode ? "#334155" : "#e2e8f0"}`,
                minHeight: "100%"
            }}>
                {output}
            </div>
        );
    }
  }

  const handleDownload = () => {
    if (!output) return
    const blob = new Blob([output], { type: 'text/plain' })
    const url = URL.createObjectURL(blob)
    const link = document.createElement('a')
    link.href = url
    link.download = `${activeTab}_analysis_output.txt`
    document.body.appendChild(link)
    link.click()
    document.body.removeChild(link)
    URL.revokeObjectURL(url)
  }

  const headerColor = activeTab === "lexical" ? "#0f4687" : "#0f4687"; 

  return (
    <div style={{ display: "flex", flexDirection: "column", height: "100%", background: darkMode ? "#1f2730" : "#ffffff" }}>
      {/* Header */}
      <div style={{
        height: "60px", padding: "0 24px", background: headerColor,
        borderBottom: `1px solid ${darkMode ? "#334155" : "#c7d2fe"}`,
        color: "#ffffff", fontWeight: "600", fontSize: "13px", textTransform: "uppercase",
        display: "flex", justifyContent: "space-between", alignItems: "center",
      }}>
        <div style={{ display: "flex", alignItems: "center", gap: "16px" }}>
          <span>{activeTab === "lexical" ? "Lexical Analysis" : "Syntax Analysis"}</span>
          
          {activeTab === "lexical" && (
            <div style={{ 
              display: "flex", alignItems: "center", background: "rgba(255,255,255,0.1)", 
              borderRadius: "4px", padding: "2px 10px", border: "1px solid rgba(255,255,255,0.2)",
              marginBottom: "-1px", height: "16px" 
            }}>
              <span style={{ fontSize: "10px", color: "#ffffff", marginRight: "4px" }}>LINE:</span>
              <div style={{ position: "relative" }}>
                  {!filterLine && <span style={{ position: "absolute", left:0, width: "100%", textAlign: "center", fontSize:"10px", pointerEvents:"none", opacity:0.8 }}>All</span>}
                  <input type="text" value={filterLine} onChange={(e) => setFilterLine(e.target.value)}
                  style={{ width: "40px", background: "transparent", border: "none", outline: "none", color: "#ffffff", fontSize: "12px", textAlign: "center", margin: 0 }} />
              </div>
            </div>
          )}
        </div>

        {output && !loading && (
          <button onClick={handleDownload} style={{
              padding: "6px 16px", background: "#ffffff", color: headerColor, border: "none",
              borderRadius: "6px", fontSize: "11px", fontWeight: "600", cursor: "pointer", textTransform: "uppercase"
            }}>
            Download
          </button>
        )}
      </div>

      <div style={{ flex: 1, overflowY: "auto", padding: "24px" }}>
        {error ? (
          <div style={{ background: "rgba(248, 81, 73, 0.1)", border: "1px solid rgba(248, 81, 73, 0.3)", padding: "16px", borderRadius: "8px", color: "#d32f2f", fontSize: "13px", marginBottom: "20px", whiteSpace: "pre-wrap" }}>
            <strong>Error:</strong> {error}
          </div>
        ) : null}

        {loading ? (
          <div style={{ textAlign: "center", padding: "40px", color: darkMode ? "#94a3b8" : "#4a89c6" }}>Running Analysis...</div>
        ) : (
          contentToRender || (
            <div style={{ textAlign: "center", padding: "40px", color: darkMode ? "#64748b" : "#94a3b8", fontSize: "13px" }}>
              {activeTab === "lexical" && filterLine ? `No data found for line ${filterLine}.` : "No analysis results generated yet."}
            </div>
          )
        )}
      </div>
    </div>
  )
}

// --- EDITOR CONTAINER COMPONENT ---
const Editor = ({ code, setCode, onRun, onClear, loading, darkMode, activeTab }) => {
  const btnColor = activeTab === "lexical" ? "#0f4687" : "#0f4687";
  
  return (
    <div style={{ height: "100%", display: "flex", flexDirection: "column" }}>
      <div
        style={{
          height: "60px", padding: "0 24px", background: darkMode ? btnColor : btnColor,
          borderBottom: `1px solid ${darkMode ? "#334155" : "#e0e7ff"}`,
          display: "flex", justifyContent: "space-between", alignItems: "center",
        }}
      >
        <div style={{ color: "#ffffff", fontWeight: "600", fontSize: "13px", textTransform: "uppercase" }}>Cnack Editor</div>
        <button
          onClick={onClear} disabled={loading}
          style={{
            padding: "6px 16px", background: "#ffffff", borderRadius: "6px", color: btnColor,
            border: "none", cursor: loading ? "not-allowed" : "pointer", fontSize: "11px", fontWeight: "600", textTransform: "uppercase"
          }}
        >
          CLEAR
        </button>
      </div>

      <div style={{ flex: 1, overflow: "hidden" }}>
        <CodeEditor value={code} onChange={setCode} disabled={loading} darkMode={darkMode} />
      </div>

      <div style={{ padding: "16px 24px", borderTop: `1px solid ${darkMode ? "#334155" : "#e0e7ff"}`, background: darkMode ? "#1f2730" : "#ffffff" }}>
        <button
          onClick={onRun} disabled={loading}
          style={{
            width: "100%", padding: "12px", background: loading ? "#94a3b8" : btnColor,
            color: "#ffffff", border: "none", borderRadius: "6px", fontWeight: "600", fontSize: "13px",
            cursor: loading ? "not-allowed" : "pointer",
          }}
        >
          {loading ? "Analyzing..." : `Run ${activeTab === "lexical" ? "Lexical" : "Syntax"} Analysis`}
        </button>
      </div>
    </div>
  )
}

// ===========================
// 3. MAIN COMPILER INTERFACE
// ===========================
const CompilerInterface = () => {
  const [activeTab, setActiveTab] = useState("syntax"); // "lexical" | "syntax"
  const [darkMode, setDarkMode] = useState(() => {
    const saved = localStorage.getItem("theme")
    return saved ? saved === "dark" : window.matchMedia("(prefers-color-scheme: dark)").matches
  })

  useEffect(() => {
    localStorage.setItem("theme", darkMode ? "dark" : "light")
  }, [darkMode])

  // Default code snippet
  const [code, setCode] = useState(`execute() {
    int x = 10;
    string msg = "Hello";
    
    assign (grade) {
        when x > 5: 1;
        otherwise: 0;
    }

    display(msg);
    exit();
}`)
  const [output, setOutput] = useState("")
  const [error, setError] = useState("")
  const [loading, setLoading] = useState(false)

  // Reset output when switching tabs
  useEffect(() => {
    setOutput("");
    setError("");
  }, [activeTab]);

  const handleRun = async () => {
    if (!code.trim()) return setError("Please enter code to analyze.")
    setLoading(true); setError(""); setOutput("")
    
    try {
      let result;
      if (activeTab === "lexical") {
        result = await runLexicalAnalysis(code)
      } else {
        result = await runSyntaxAnalysis(code)
      }
      
      if (result.success === false) {
        // Return output even on failure (e.g. detailed syntax errors)
        if (result.output) {
            setOutput(result.output);
            // Grab the first line as a summary error
            setError(result.output.split('\n')[0]); 
        } else {
            setError("Analysis failed. Check your code.");
        }
      } else {
        setOutput(result.output)
      }

    } catch (err) { 
      setError(err.message) 
    } finally { 
      setLoading(false) 
    }
  }

  const primaryColor = activeTab === "lexical" ? "#0f4687" : "#0f4687";

  // Tab Button Style
  const getTabStyle = (tabName) => ({
    padding: "8px 16px",
    background: activeTab === tabName ? "rgba(255,255,255,0.2)" : "transparent",
    border: "none",
    color: "#fff",
    borderRadius: "6px",
    cursor: "pointer",
    fontSize: "12px",
    fontWeight: "600",
    transition: "0.2s"
  });

  return (
    <div style={{
      display: "flex", flexDirection: "column", width: "100vw", height: "100vh",
      background: darkMode ? "#28313b" : "#d8e6f4ff", transition: "all 0.3s ease"
    }}>
      {/* App Header with Tabs */}
      <div style={{
        padding: "16px 24px", background: darkMode ? "#1f2730" : "#f8fafc",
        borderBottom: `1px solid ${darkMode ? "#334155" : "#e0e7ff"}`,
        display: "flex", justifyContent: "space-between", alignItems: "center"
      }}>
        <div style={{ display: "flex", alignItems: "center", gap: "8px" }}> 
          <img src="Cnack_nobg.png" alt="Logo" style={{ height: "40px" }} />
          <h1 style={{ margin: 0, fontSize: "18px", color: darkMode ? "#f8fafc" : "#0f4687" }}>
             Cnack Mini Compiler
          </h1>
          
          <div onClick={() => setDarkMode(!darkMode)}
            style={{
              marginLeft: "12px", width: "42px", height: "22px", background: darkMode ? primaryColor : "#cbd5e1",
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

        {/* Tab Switcher */}
        <div style={{ display: "flex", gap: "8px", background: primaryColor, padding: "4px", borderRadius: "8px" }}>
            <button onClick={() => setActiveTab("lexical")} style={getTabStyle("lexical")}>
                LEXICAL
            </button>
            <button onClick={() => setActiveTab("syntax")} style={getTabStyle("syntax")}>
                SYNTAX
            </button>
        </div>
      </div>

      <div style={{ display: "grid", gridTemplateColumns: "1fr 1fr", flex: 1, gap: "16px", padding: "16px", overflow: "hidden" }}>
        <div style={{ background: darkMode ? "#1f2730" : "#ffffff", borderRadius: "8px", border: `1px solid ${darkMode ? "#334155" : "#b9babdff"}`, overflow: "hidden" }}>
          <Editor code={code} setCode={setCode} onRun={handleRun} onClear={() => setCode("")} loading={loading} darkMode={darkMode} activeTab={activeTab} />
        </div>
        <div style={{ background: darkMode ? "#1f2730" : "#ffffff", borderRadius: "8px", border: `1px solid ${darkMode ? "#334155" : "#b9babdff"}`, overflow: "hidden" }}>
          <Output output={output} error={error} loading={loading} darkMode={darkMode} activeTab={activeTab} />
        </div>
      </div>
    </div>
  )
}

// ===========================
// 4. MAIN APP WRAPPER
// ===========================
const App = () => {
  const [hasStarted, setHasStarted] = useState(false);

  // If the user hasn't clicked start, show HomePage
  if (!hasStarted) {
    return <HomePage onStart={() => setHasStarted(true)} />;
  }

  // Otherwise, show the main Compiler Interface
  return <CompilerInterface />;
}

export default App