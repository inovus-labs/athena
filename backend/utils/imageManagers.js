// import geminiModel from "../config/geminiConfig.js"

import fs from "node:fs"
import dotenv from "dotenv"

// Config .env
dotenv.config()

// Image to text
const imageToText = async (image) => {
    if (!image) {
        return { error: "No image found!" }
    }

    const geminiRes = await fetch(
        `https://generativelanguage.googleapis.com/v1beta/models/gemini-2.5-flash:generateContent?key=${process.env.GEMINI_API_KEY}`,
        {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({
                contents: [
                    {
                        parts: [
                            {
                                text:
                                    "You are ATHENA, an AI assistant for elderly and visually impaired users. " +
                                    "Describe the image clearly and simply in one or two short sentences. " +
                                    "Prioritize safety, obstacles, people, and notable items. Avoid speculation. " +
                                    "Avoid technical terms, be friendly, and sound natural, like speaking to a companion.",
                            },
                            { inlineData: { data: image, mimeType: "image/jpeg" } },
                        ],
                    },
                ],
                generationConfig: { temperature: 0.5, maxOutputTokens: 150 },
            }),
        }
    )

    if (!geminiRes.ok) throw new Error("Gemini: " + (await geminiRes.text()))
    const geminiData = await geminiRes.json()

    const description =
        (geminiData?.candidates?.[0]?.content?.parts || [])
            .map((p) => p.text || "")
            .join(" ")
            .trim() || "I could not describe the image."

    return description
}

// Image to text and speech
const imageToTextSpeech = async (image) => {
    try {
        if (!image) {
            console.error("No image found")
            return {
                message: "No image found!"
            }
        }

        const description = await imageToText(image)

        const tokenRes = await fetch(
            "https://centralindia.api.cognitive.microsoft.com/sts/v1.0/issueToken",
            {
                method: "POST",
                headers: {
                    "Ocp-Apim-Subscription-Key": process.env.AZURE_SPEECH_KEY,
                    "Content-Length": "0",
                },
            }
        )
        if (!tokenRes.ok) throw new Error("Azure token: " + (await tokenRes.text()))
        const token = await tokenRes.text()

        const ssml =
            `<speak version="1.0" xml:lang="en-IN"><voice name="en-IN-NeerjaNeural">` +
            description.replace(/&/g, "&amp").replace(/</g, "&lt") +
            `</voice></speak>`

        const ttsRes = await fetch(
            "https://centralindia.tts.speech.microsoft.com/cognitiveservices/v1",
            {
                method: "POST",
                headers: {
                    Authorization: "Bearer " + token,
                    "Content-Type": "application/ssml+xml",
                    "X-Microsoft-OutputFormat": "audio-16khz-32kbitrate-mono-mp3",
                    Accept: "audio/mpeg",
                    "User-Agent": "athena-ultra-simple",
                },
                body: ssml,
            }
        )
        if (!ttsRes.ok) throw new Error("Azure TTS: " + (await ttsRes.text()))
        const audio = Buffer.from(await ttsRes.arrayBuffer())

        fs.writeFileSync("output.mp3", audio)
        console.log("Saved output.mp3\nGemini:", description)

        return {
            text: description,
            audioFile: audio
        };

    } catch (error) {
        console.log("No image", error.message)
    }
}

export { imageToText, imageToTextSpeech }