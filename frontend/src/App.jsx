"use client"

import { useState } from "react"

// API Helper
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

// -------------------- Code Editor with Line Numbers --------------------
const CodeEditor = ({ value, onChange, disabled }) => {
  const handleKeyDown = (e) => {
    if (e.key === "Tab") {
      e.preventDefault()
      const start = e.target.selectionStart
      const end = e.target.selectionEnd

      const newValue = value.substring(0, start) + "    " + value.substring(end)
      onChange(newValue)

      setTimeout(() => {
        e.target.selectionStart = e.target.selectionEnd = start + 4
      }, 0)
    }
  }

  const lines = value.split("\n").length
  const lineNumbers = Array.from({ length: lines }, (_, i) => i + 1).join("\n")

  return (
    <div style={{ display: "flex", height: "100%", background: "#f5f1ed" }}>
      {/* Line Numbers */}
      <div
        style={{
          width: "50px",
          padding: "20px 0",
          background: "#ede8e3",
          borderRight: "1px solid #d4c4b0",
          fontFamily: '"Fira Code", "Consolas", monospace',
          fontSize: "13px",
          lineHeight: "1.6",
          color: "#a89884",
          textAlign: "right",
          paddingRight: "12px",
          overflowY: "hidden",
          userSelect: "none",
        }}
      >
        <pre style={{ margin: 0, fontSize: "13px", lineHeight: "1.6" }}>{lineNumbers}</pre>
      </div>

      {/* Editor */}
      <textarea
        value={value}
        onChange={(e) => onChange(e.target.value)}
        onKeyDown={handleKeyDown}
        disabled={disabled}
        placeholder="// write code here"
        spellCheck={false}
        style={{
          flex: 1,
          padding: "20px",
          fontFamily: '"Fira Code", "Consolas", monospace',
          fontSize: "13px",
          lineHeight: "1.6",
          resize: "none",
          border: "none",
          outline: "none",
          background: "#f5f1ed",
          color: "#3d2817",
          overflowY: "auto",
          transition: "background 0.2s",
        }}
      />
    </div>
  )
}

// -------------------- Parsing Output --------------------
const parseTokenOutput = (output) => {
  const lines = output.split("\n")
  const tokens = []
  let startParsing = false

  for (const line of lines) {
    if (line.includes("-------|")) {
      startParsing = true
      continue
    }

    if (startParsing && line.trim()) {
      const parts = line.split("|").map((p) => p.trim())
      if (parts.length === 3) {
        tokens.push({
          line: parts[0],
          type: parts[1],
          lexeme: parts[2],
        })
      }
    }
  }

  return tokens
}

// -------------------- Output Component --------------------
const Output = ({ output, error, loading }) => {
  const tokens = output ? parseTokenOutput(output) : []

  return (
    <div
      style={{
        display: "flex",
        flexDirection: "column",
        height: "100%",
        background: "#f5f1ed",
      }}
    >
      {/* Header */}
      <div
        style={{
          padding: "16px 24px",
          background: "#d4c4b0",
          borderBottom: "1px solid #c4b09c",
          color: "#5c4a38",
          fontWeight: "600",
          fontSize: "13px",
          textTransform: "uppercase",
          letterSpacing: "0.5px",
          textAlign: "center",
        }}
      >
        Lexical Analysis
      </div>

      {/* Scrollable Content */}
      <div
        style={{
          flex: 1,
          overflowY: "auto",
          padding: "24px",
        }}
      >
        {error ? (
          <div
            style={{
              background: "rgba(248, 81, 73, 0.1)",
              border: "1px solid rgba(248, 81, 73, 0.3)",
              padding: "16px",
              borderRadius: "8px",
              color: "#d32f2f",
              fontSize: "13px",
            }}
          >
            <strong style={{ display: "block", marginBottom: "8px" }}>Error</strong>
            <div>{error}</div>
          </div>
        ) : loading ? (
          <div style={{ textAlign: "center", padding: "40px", color: "#a89884" }}>
            <div style={{ fontSize: "13px", fontWeight: "500" }}>Analyzing...</div>
          </div>
        ) : tokens.length > 0 ? (
          <div
            style={{
              background: "#fff9f3",
              borderRadius: "8px",
              overflow: "hidden",
              border: "1px solid #d4c4b0",
            }}
          >
            {/* Table Header */}
            <div
              style={{
                display: "grid",
                gridTemplateColumns: "repeat(3, 1fr)",
                padding: "16px 20px",
                background: "#d4c4b0",
                fontWeight: "600",
                fontSize: "12px",
                color: "#5c4a38",
                textTransform: "uppercase",
                borderBottom: "1px solid #c4b09c",
                letterSpacing: "0.5px",
                textAlign: "center",
              }}
            >
              <div>Line</div>
              <div>Type</div>
              <div>Lexeme</div>
            </div>

            {/* Body */}
            <div>
              {tokens.map((t, i) => (
                <div
                  key={i}
                  style={{
                    display: "grid",
                    gridTemplateColumns: "repeat(3, 1fr)",
                    padding: "12px 20px",
                    background: i % 2 === 0 ? "#f5f1ed" : "#fff9f3",
                    borderBottom: "1px solid #e0d4c4",
                    fontSize: "13px",
                    transition: "background 0.15s",
                    textAlign: "center",
                  }}
                  onMouseEnter={(e) => {
                    e.currentTarget.style.background = "#e8dcc8"
                  }}
                  onMouseLeave={(e) => {
                    e.currentTarget.style.background = i % 2 === 0 ? "#f5f1ed" : "#fff9f3"
                  }}
                >
                  <div style={{ color: "#8b7355" }}>{t.line}</div>
                  <div style={{ color: "#9d7e63" }}>{t.type}</div>
                  <div style={{ color: "#5c4a38" }}>{t.lexeme}</div>
                </div>
              ))}
            </div>
          </div>
        ) : (
          <div style={{ textAlign: "center", padding: "40px", color: "#a89884", fontSize: "13px" }}>
            No analysis yet. Enter code and click "Analyze" to get started.
          </div>
        )}
      </div>
    </div>
  )
}

// -------------------- Editor Component --------------------
const Editor = ({ code, setCode, onRun, onClear, loading }) => {
  return (
    <div style={{ height: "100%", display: "flex", flexDirection: "column" }}>
      {/* Header */}
      <div
        style={{
          padding: "16px 24px",
          background: "#fff9f3",
          borderBottom: "1px solid #d4c4b0",
          display: "flex",
          justifyContent: "space-between",
          alignItems: "center",
        }}
      >
        <div
          style={{
            color: "#5c4a38",
            fontWeight: "600",
            fontSize: "13px",
            textTransform: "uppercase",
            letterSpacing: "0.5px",
          }}
        >
          Code Input
        </div>

        <button
          onClick={onClear}
          disabled={loading}
          style={{
            padding: "6px 12px",
            background: "#d4c4b0",
            borderRadius: "6px",
            color: "#5c4a38",
            border: "1px solid #c4b09c",
            cursor: loading ? "not-allowed" : "pointer",
            fontSize: "12px",
            fontWeight: "500",
            transition: "all 0.15s",
            opacity: loading ? 0.5 : 1,
          }}
          onMouseEnter={(e) => {
            if (!loading) {
              e.target.style.background = "#c4b09c"
              e.target.style.color = "#3d2817"
            }
          }}
          onMouseLeave={(e) => {
            if (!loading) {
              e.target.style.background = "#d4c4b0"
              e.target.style.color = "#5c4a38"
            }
          }}
        >
          Clear
        </button>
      </div>

      {/* Code Editor */}
      <div style={{ flex: 1, overflow: "auto" }}>
        <CodeEditor value={code} onChange={setCode} disabled={loading} />
      </div>

      {/* Analyze Button */}
      <div style={{ padding: "16px 24px", borderTop: "1px solid #d4c4b0" }}>
        <button
          onClick={onRun}
          disabled={loading}
          style={{
            width: "100%",
            padding: "12px",
            background: loading ? "rgba(139, 109, 79, 0.3)" : "#8b6d4f",
            color: "#fff9f3",
            border: "none",
            borderRadius: "6px",
            fontWeight: "600",
            fontSize: "13px",
            cursor: loading ? "not-allowed" : "pointer",
            transition: "all 0.15s",
          }}
          onMouseEnter={(e) => {
            if (!loading) {
              e.target.style.background = "#a0835d"
            }
          }}
          onMouseLeave={(e) => {
            if (!loading) {
              e.target.style.background = "#8b6d4f"
            }
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
  const [code, setCode] = useState("// Cnack Mini Compiler\nint main() {\n    int x = 10;\n    return 0;\n}")
  const [output, setOutput] = useState("")
  const [error, setError] = useState("")
  const [loading, setLoading] = useState(false)

  const handleRun = async () => {
    if (!code.trim()) {
      setError("Please enter some code to analyze")
      setOutput("")
      return
    }

    setLoading(true)
    setError("")
    setOutput("")

    try {
      const result = await runLexicalAnalysis(code)
      setOutput(result.output)
    } catch (err) {
      setError(err.message)
    } finally {
      setLoading(false)
    }
  }

  const handleClear = () => {
    setCode("")
    setOutput("")
    setError("")
  }

  return (
    <div
      style={{
        display: "flex",
        flexDirection: "column",
        width: "100vw",
        height: "100vh",
        fontFamily: '-apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif',
        background: "#fff9f3",
        overflow: "hidden",
      }}
    >
      {/* Header */}
      <div
        style={{
          padding: "16px 24px",
          background: "#f5f1ed",
          borderBottom: "1px solid #d4c4b0",
          display: "flex",
          justifyContent: "space-between",
          alignItems: "center",
        }}
      >
        <h1 style={{ margin: "0", fontSize: "18px", fontWeight: "700", color: "#5c4a38", letterSpacing: "-0.5px" }}>
          Cnack Mini Compiler
        </h1>
        <div style={{ fontSize: "12px", color: "#a89884" }}>Lexical Analysis</div>
      </div>

      {/* Two-panel layout */}
      <div
        style={{
          display: "grid",
          gridTemplateColumns: "1fr 1fr",
          flex: 1,
          overflow: "hidden",
          gap: "16px",
          padding: "16px",
        }}
      >
        {/* Left Panel - Editor */}
        <div
          style={{
            background: "#fff9f3",
            borderRadius: "8px",
            display: "flex",
            flexDirection: "column",
            minWidth: "0",
            border: "1px solid #d4c4b0",
            overflow: "hidden",
          }}
        >
          <Editor code={code} setCode={setCode} onRun={handleRun} onClear={handleClear} loading={loading} />
        </div>

        {/* Right Panel - Output */}
        <div
          style={{
            background: "#fff9f3",
            borderRadius: "8px",
            display: "flex",
            flexDirection: "column",
            minWidth: "0",
            border: "1px solid #d4c4b0",
            overflow: "hidden",
          }}
        >
          <Output output={output} error={error} loading={loading} />
        </div>
      </div>
    </div>
  )
}

export default App
