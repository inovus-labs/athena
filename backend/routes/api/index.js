import express from "express"
import { imageInfo } from "../../controllers/imageControllers.js"

const router = express.Router()

router.get("/", (req, res) => {
    res.status(200).json({
        status: 200,
        message: "Athena API V1"
    })
})

// GET image by filename
router.get("/getImageInfo/:filename", imageInfo)

export default router