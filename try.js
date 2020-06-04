const {PorcupineDetector, PorcupineKeyword} = require('./lib')
const Spawn = require('child_process').spawn
const Block =  require('block-stream2')

const PORCUPINE_PATH	= 'D:/Projects/Assistant/pv-porcupine/Porcupine'
const LIBRARY_PATH		= PORCUPINE_PATH + '/lib/windows/amd64/libpv_porcupine.dll'
const MODEL_PATH		= PORCUPINE_PATH + '/lib/common/porcupine_params.pv'
const KEYWORD_PATH		= PORCUPINE_PATH + '/resources/keyword_files/windows/picovoice_windows.ppn'
const SENSITIVITY		= 0.7

function main() {
	const keyword = new PorcupineKeyword('bumblebee', KEYWORD_PATH, SENSITIVITY)

	const detector = new PorcupineDetector({
		library: LIBRARY_PATH,
		model: MODEL_PATH,
		keywords: [keyword]
	})

	detector.on('wakeword', keyword => console.log('Keyword detected:', keyword))

	const FRAME_LENGTH	= detector.frameLength
	const SAMPLE_RATE	= detector.sampleRate
	const BIT_LENGTH 	= 2
	const CHANNELS 		= 1
	const ENDIANNESS 	= 'little'

	console.log( "version       : %s", detector.version )
	console.log( "frame length  : %d", FRAME_LENGTH )
	console.log( "sample rate   : %d", SAMPLE_RATE )

	const command = 'sox'
	const args = [
		'--no-show-progress',
		'--buffer=' + FRAME_LENGTH * BIT_LENGTH,
		'--channels=' + CHANNELS,
		'--endian=' + ENDIANNESS,
		'--bits=' + BIT_LENGTH * 8,
		'--rate=' + SAMPLE_RATE,
		'--default-device',
		'--type=raw',
		'-',
		// 'gain',
		// '-n',
		// '3'
	]

	console.log(args)
	console.log('Listening...')

	const capture = Spawn(command, args)

	capture.on('end', () => console.log('Ended'))

	capture.stderr.pipe(process.stderr)

	capture.stdout
		.pipe(new Block( FRAME_LENGTH * 2 ))
		.pipe(detector)
}

main()