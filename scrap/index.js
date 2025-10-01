
import "dotenv/config";

import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";

(async () => {
  try {
    const b64 = fs
      .readFileSync(path.join(path.dirname(fileURLToPath(import.meta.url)), "sample.jpeg"))
      .toString("base64");

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
                    "You are an assistant for elderly and visually impaired users. " +
                    "Describe the image clearly and simply in one or two short sentences. " +
                    "Prioritize safety, obstacles, people, and notable items. Avoid speculation.",
                },
                { inlineData: { data: b64, mimeType: "image/jpeg" } },
              ],
            },
          ],
          generationConfig: { temperature: 0.6, maxOutputTokens: 200 },
        }),
      }
    );
    if (!geminiRes.ok) throw new Error("Gemini: " + (await geminiRes.text()));
    const geminiData = await geminiRes.json();
    const description =
      (geminiData?.candidates?.[0]?.content?.parts || [])
        .map((p) => p.text || "")
        .join(" ")
        .trim() || "I could not describe the image.";

    const tokenRes = await fetch(
      "https://centralindia.api.cognitive.microsoft.com/sts/v1.0/issueToken",
      { method: "POST", headers: { "Ocp-Apim-Subscription-Key": process.env.AZURE_SPEECH_KEY, "Content-Length": "0" } }
    );
    if (!tokenRes.ok) throw new Error("Azure token: " + (await tokenRes.text()));
    const token = await tokenRes.text();

    const ssml =
      `<speak version="1.0" xml:lang="en-US"><voice name="en-US-JennyNeural">` +
      description.replace(/&/g, "&amp;").replace(/</g, "&lt;") +
      `</voice></speak>`;

    const ttsRes = await fetch("https://centralindia.tts.speech.microsoft.com/cognitiveservices/v1", {
      method: "POST",
      headers: {
        Authorization: "Bearer " + token,
        "Content-Type": "application/ssml+xml",
        "X-Microsoft-OutputFormat": "audio-16khz-32kbitrate-mono-mp3",
        Accept: "audio/mpeg",
        "User-Agent": "athena-ultra-simple",
      },
      body: ssml,
    });
    if (!ttsRes.ok) throw new Error("Azure TTS: " + (await ttsRes.text()));
    const audio = Buffer.from(await ttsRes.arrayBuffer());

    fs.writeFileSync("output.mp3", audio);
    console.log("Saved output.mp3\nGemini:", description);

  } catch (e) {
    console.error("✖ Error:", e?.message || e);
    process.exit(1);
  }
})();
