console.log("RUNTIME __dirname:", __dirname);

const express = require('express');
const cors = require('cors');
const bodyParser = require('body-parser');
const path = require('path');
const fs = require('fs');
const { spawn } = require('child_process');

const app = express();
const PORT = 3001; 

// Middleware
app.use(cors());
app.use(bodyParser.json());
app.use(bodyParser.text());

// -----------------------------
//  PATHS TO EXECUTABLES
// -----------------------------
const BIN_DIR = path.join(__dirname, 'bin');
const LEXER_PATH = path.join(BIN_DIR, 'lexer.exe');
const PARSER_PATH = path.join(BIN_DIR, 'parser.exe'); // NEW: Path for Syntax Analyzer

// Root endpoint
app.get('/', (req, res) => {
  res.json({ message: 'Cnack Compiler API is running!' });
});

// -----------------------------
//  1. LEXICAL ANALYSIS ENDPOINT
// -----------------------------
app.post('/lexical', (req, res) => {
  handleProcess(req, res, LEXER_PATH, "Lexer");
});

// -----------------------------
//  2. SYNTAX ANALYSIS ENDPOINT (NEW)
// -----------------------------
app.post('/syntax', (req, res) => {
  handleProcess(req, res, PARSER_PATH, "Parser");
});

// -----------------------------
//  HELPER FUNCTION
// -----------------------------
// Reusable function to handle both Lexer and Parser processes
function handleProcess(req, res, executablePath, processName) {
  const { code } = req.body;

  // Validate user input 
  if (!code || code.trim() === '') {
    return res.status(400).json({
      error: 'Empty code: Please enter some code to analyze.'
    });
  }

  // Ensure executable exists 
  if (!fs.existsSync(executablePath)) {
    return res.status(500).json({
      error: `${processName} executable NOT FOUND at: ${executablePath}.\nPlease place the compiled .exe inside the /bin folder.`
    });
  }

  // Spawn the process (Lexer or Parser) 
  const child = spawn(executablePath);

  let output = '';
  let errorOutput = '';

  // Capture Standard Output
  child.stdout.on('data', (data) => {
    output += data.toString();
  });

  // Capture Standard Error
  child.stderr.on('data', (data) => {
    errorOutput += data.toString();
  });

  // Handle Process Closure
  child.on('close', (code) => {
    // If the process had stderr output, treat it as an error message
    // Note: Some compilers print warnings to stderr, so you might want to adjust this logic depending on your C code's behavior.
    if (errorOutput && code !== 0) { 
        return res.json({ 
            success: false, 
            output: errorOutput, // Send the error message from C code back to frontend
            type: 'error'
        });
    }

    return res.json({
      success: true,
      output: output || 'No output generated.',
      type: 'success'
    });
  });

  // Write code to STDIN of the C program 
  child.stdin.write(code);
  child.stdin.end();
}

// -----------------------------
//  ERROR HANDLER
// -----------------------------
app.use((err, req, res, next) => {
  console.error('Server error:', err);
  return res.status(500).json({
    error: 'Internal server error.'
  });
});

// -----------------------------
//  START SERVER
// -----------------------------
app.listen(PORT, () => {
  console.log(`\n🚀 Cnack Compiler API running at: http://localhost:${PORT}`);
  console.log(`📁 Bin Directory: ${BIN_DIR}`);
  console.log(`   Expecting Lexer at:  ${LEXER_PATH}`);
  console.log(`   Expecting Parser at: ${PARSER_PATH}`);
  console.log(`✅ Ready to accept requests!\n`);
});