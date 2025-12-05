# 🍪 Cnack Mini Compiler

A **web-based lexical analyzer** for the CNACK programming language. Input your code and see it broken down into tokens (keywords, identifiers, operators, numbers, etc.).

## 📋 Prerequisites

- **Node.js**
- **GCC** (for compiling the lexer if you make changes)

### BACKEND

Step 1: Go to backend folder

```cmd
cd backend
```

Step 2: Start the backend server

```cmd
node server.js
```

### FRONTEND

Step 1: Go to frontend folder

```cmd
cd frontend
```

Step 2: Run the frontend

```cmd
npm run dev
```

## 🔧 Making Changes to the Lexer

If you modify `lexer.c`:

Step 1: Recompile the lexer

```cmd
gcc lexer.c -o bin/lexer
```

Step 2: Restart the backend server

```cmd
node server.js
```

## 📖 How to Use

1. Open the website.
2. Type or paste CNACK code in the input area.
3. Click "Analyze"
4. View the token results (line number, token type, lexeme).

---

<p align="center">✨ The familiar flavor of C, now with extra crunch ✨</p>
