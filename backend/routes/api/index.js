import express from "express"

const router = express.Router()

router.get("/", (req, res) => {
    res.status(200).json({
        status: 200,
        message: "Athena API V1"
    })
})

export default router