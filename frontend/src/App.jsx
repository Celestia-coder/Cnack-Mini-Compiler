"use client"

import { useState, useRef, useCallback, useEffect } from "react" 

const API_URL = "http://localhost:3001"

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

// -------------------- Code Editor Component --------------------
const CodeEditor = ({ value, onChange, disabled, darkMode }) => {
  const editorRef = useRef(null)
  const lineNumbersRef = useRef(null)

  const handleScroll = useCallback(() => {
    if (editorRef.current && lineNumbersRef.current) {
      lineNumbersRef.current.scrollTop = editorRef.current.scrollTop
    }
  }, [])

  const handleKeyDown = (e) => {
    const start = e.target.selectionStart
    const end = e.target.selectionEnd

    if (e.key === "Tab") {
      e.preventDefault()
      const newValue = value.substring(0, start) + "    " + value.substring(end)
      onChange(newValue)
      setTimeout(() => {
        e.target.selectionStart = e.target.selectionEnd = start + 4
      }, 0)
    }

    if (e.key === "Enter") {
      e.preventDefault()
      const lines = value.substring(0, start).split('\n');
      const currentLine = lines[lines.length - 1];
      const match = currentLine.match(/^(\s*)/);
      let indent = match ? match[0] : ''; 
      const trimmedLine = currentLine.trim();
    
      if (trimmedLine.endsWith('{') || trimmedLine.endsWith('(') || trimmedLine.endsWith('[')) {
        indent += '    ';
      }

      const newValue = value.substring(0, start) + "\n" + indent + value.substring(end)
      onChange(newValue)
      setTimeout(() => {
        e.target.selectionStart = e.target.selectionEnd = start + 1 + indent.length
      }, 0)
    }
  }

  const lines = value.split("\n").length
  const lineNumbers = Array.from({ length: lines }, (_, i) => i + 1).join("\n")

  const sharedStyles = {
    fontFamily: '"Fira Code", "Consolas", monospace',
    fontSize: "13px",
    lineHeight: "21px",    
    paddingTop: "20px",
    paddingBottom: "20px", 
  }

  return (
    <div style={{ display: "flex", height: "100%", background: darkMode ? "#1f2730" : "#ffffff", position: "relative" }}>
      <div
        ref={lineNumbersRef}
        style={{
          ...sharedStyles,
          width: "60px",
          background: darkMode ? "#28313b" : "#f8fafc",
          borderRight: `1px solid ${darkMode ? "#334155" : "#e0e7ff"}`,
          color: darkMode ? "#64748b" : "#4a89c6",
          textAlign: "right",
          paddingRight: "16px",
          overflow: "hidden", 
          userSelect: "none",
        }}
      >
        <pre style={{ margin: 0, ...sharedStyles, paddingTop: 0 }}>{lineNumbers}</pre>
      </div>

      <textarea
        ref={editorRef}
        value={value}
        onChange={(e) => onChange(e.target.value)}
        onKeyDown={handleKeyDown}
        onScroll={handleScroll}
        disabled={disabled}
        placeholder="// write code here"
        spellCheck={false}
        style={{
          ...sharedStyles,
          flex: 1,
          paddingLeft: "24px",
          paddingRight: "20px",
          resize: "none",
          border: "none",
          outline: "none",
          background: "transparent",
          color: darkMode ? "#e2e8f0" : "#0f4687",
          overflowY: "auto",
          whiteSpace: "pre",
          overflowX: "auto",
        }}
      />
    </div>
  )
}

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
        <div style={{ display: "flex", alignItems: "center", gap: "12px" }}>
          <img src="Cnack_nobg.png" alt="Logo" style={{ height: "45px" }} />
          <h1 style={{ margin: 0, fontSize: "18px", color: darkMode ? "#f8fafc" : "#0f4687" }}>Cnack Mini Compiler</h1>
          
          {/* Toggle Switch */}
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