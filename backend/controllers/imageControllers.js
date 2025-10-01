import path from "path";
import multer from "multer";

// Setup multer storage
const storage = multer.diskStorage({
    destination: (req, file, cb) => { // `cb` stands for callback
        cb(null, 'storage/') // where to store the files
    },
    filename: (req, file, cb) => {
        cb(null, Date.now() + path.extname(file.originalname)) // gives the file a unique name
    }
})

export const upload = multer({ storage: storage });

export const imageInfo = async (req, res) => {

    upload.array('images')(req, res, async (error) => {
        if (error) {
            return res.status(400).json({
                status: 400,
                message: "Error uploading images",
                error: error.message
            })
        }
        try {
            const newImage = {
                "images": req.files
            }
            if (newImage) {
                return res.status(200).json({
                    status: 200,
                    message: "Image uploaded successfully",
                    images: req.files,
                })
            } else {
                return res.status(400).json({
                    status: 400,
                    message: "No mail found!"
                })
            }
        } catch (error) {
            console.error("Error: something went wrong,", error.message)
            return res.status(500).json({
                status: 500,
                message: "Internal server error",
                error: error.message
            })
        }
    })
};
