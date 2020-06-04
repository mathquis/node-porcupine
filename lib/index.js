const Stream					= require('stream')
const File						= require('fs')
const Path						= require('path')
const Binary					= require('node-pre-gyp')
const binding_path				= Binary.find(Path.resolve(Path.join(__dirname,'../package.json')))
const {PorcupineNativeDetector}	= require(binding_path)

class PorcupineKeyword {
	constructor(keyword, path, sensitivity) {
		this.keyword = keyword
		const keywordFile = path || ''
		this.keywordFile = Path.resolve(process.cwd(), keywordFile)
		const stat = File.statSync(this.keywordFile)
		if ( !stat.isFile() ) throw new Error(`Keyword file "${path}" should be an existing file`)
		this.sensitivity = Math.max(0, Math.min(1, sensitivity))
		if ( isNaN(this.sensitivity) ) throw new Error('Sensitivity should be a number between 0 and 1')
	}
}

class PorcupineDetector extends Stream.Writable {
	constructor(options) {
		options || (options = {})
		super()

		// Library
		const library = options.library || ''
		this.library = Path.resolve(process.cwd(), library)
		const stat_lib = File.statSync(this.library)
		if ( !stat_lib.isFile() ) throw new Error(`Library file "${library}" should be an existing file`)

		// Model
		const model = options.model || ''
		this.model = Path.resolve(process.cwd(), model)
		const stat_model = File.statSync(this.model)
		if ( !stat_model.isFile() ) throw new Error(`Model file "${model}" should be an existing file`)

		// Keywords
		if ( !options.keywords ) throw new Error(`Missing keyword file`)
		if ( !Array.isArray(options.keywords) ) options.keywords = [options.keywords]

		this.keywords		= options.keywords
		this.keywordFiles 	= []
		this.sensitivities	= []
		options.keywords.forEach(keyword => {
			if ( !(keyword instanceof PorcupineKeyword) ) throw new Error(`Keyword should be a PorcupineKeyword instance`)
			this.keywordFiles.push(keyword.keywordFile)
			this.sensitivities.push(keyword.sensitivity)
		})

		// Create detector
		this._detector = new PorcupineNativeDetector(this.library, this.model, this.keywordFiles[0], this.sensitivities[0])
	}

	process(chunk) {
		const result = this._detector.process(chunk)
		if ( result >= 0 ) {
			// Match keyword
			const keyword = this.keywords[result]
			if ( keyword ) {
				this.emit('wakeword', keyword)
			}
		}
	}

	get frameLength() {
		return this._detector.getFrameLength()
	}

	get sampleRate() {
		return this._detector.getSampleRate()
	}

	get version() {
		return this._detector.getVersion()
	}

	_write(chunk, encoding, callback) {
		// console.log(chunk)
		this.process(chunk)
		return callback()
	}
}

module.exports = {
	PorcupineDetector,
	PorcupineKeyword
}