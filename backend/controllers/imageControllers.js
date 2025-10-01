import path from 'path'
import fs from 'fs/promises'
import { fileURLToPath } from 'url'

const __filename = fileURLToPath(import.meta.url)
const __dirname = path.dirname(__filename)

export const imageInfo = async (req, res) => {
    try {
        const filename = req.params.filename
        const imagePath = path.join(__dirname, '..', 'images', filename) // Adjust path if needed

        // Read file with async/await
        const data = await fs.readFile(imagePath)

        // Detect MIME type
        const ext = path.extname(filename).toLowerCase()
        let contentType = 'application/octet-stream'
        if (ext === '.jpg' || ext === '.jpeg') {
            contentType = 'image/jpeg'
        } else if (ext === '.png') {
            contentType = 'image/png'
        } else if (ext === '.gif') {
            contentType = 'image/gif'
        }

        // Send the image
        res.setHeader('Content-Type', contentType)
        return res.send(data)
    } catch (err) {
        console.error('Error reading image:', err)
        return res.status(404).json({
            status: 404,
            message: 'Image not found'
        })
    }
}
