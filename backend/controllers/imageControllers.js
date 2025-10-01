import multer from "multer"
import imageManager from "../utils/imageManagers.js"

// Setup multer storage
const storage = multer.memoryStorage();

export const upload = multer({ storage });

export const imageInfo = async (req, res) => {
    upload.array("images")(req, res, async (error) => {
        if (error) {
            return res.status(400).json({
                status: 400,
                message: error.message
            });
        }

        if (!req.files || req.files.length === 0) {
            return res.status(400).json({ status: 400, message: "No image uploaded!" });
        }

        // Directly use buffer without disk read
        const uploadedImages = req.files.map((file) => ({
            originalName: file.originalname,
            size: file.size,
            mimetype: file.mimetype,
        }));

        const base64Image = req.files[0].buffer.toString("base64");
        const imgRes = await imageManager(base64Image);

        res.status(200).json({
            status: 200,
            message: "Images uploaded successfully",
            // images: uploadedImages,
            imgRes
        });
    });
};
