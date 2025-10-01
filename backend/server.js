import express from "express"
import dotenv from "dotenv"
import routes from "./routes/index.js"

const app = express()

// Config .env
dotenv.config()

// Base URL
app.use("/", routes)

app.listen((process.env.PORT || 5000), () => {
    console.log(`\n🚀 Server listening on port: ${process.env.PORT || 5000}`)
})