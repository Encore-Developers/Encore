# Encore Chart Format
To work in Encore, a chart requires the following:
- Properly formatted `info.json`
- At least 1 `backing.ogg`
- At least 1 valid instrument track in `<name>.mid`

Details about these files and the format will be noted below.

## info.json
A fully fleshed out `info.json` file will look similar to the following:
```json
{   
	"title": "Through the Fire and Flames",    
	"artist": "DragonForce",    
	"preview_start_time": 41727,    
	"release_year": "2006",    
	"source": "rb3dlc",    
	"album": "Inhuman Rampage",    
	"loading_phrase": "Woah it's an encore in here!",    
	"genres": [    
		"Power Metal"    
	],    
	"charters": [
		"Harmonix",
		"TheGuitarHeroNerd",
		"Gwarb"
	],
	"length": 449,
	"icon_drums": "Drum",
	"icon_bass": "Bass",
	"icon_guitar": "Guitar",
	"icon_vocals": "Vocals",
	"diff": {
		"drums": -1,
		"bass": 5,
		"guitar": 6,
		"vocals": 4,
		"plastic_drums": -1,
		"plastic_bass": 5,
		"plastic_guitar": 6,
		"pitched_vocals": 4
	},
	"midi": "notes.mid",
	"art": "cover.png",
	"stems": {
		"drums": [
			"drums_1.ogg",
			"drums_2.ogg",
			"drums_3.ogg"
		],
		"bass": "bass.ogg",
		"lead": "lead.ogg",
		"vocals": "vocals.ogg",
		"backing": [
			"backing.ogg",
			"keys.ogg"
		]
	}
}
```

The minimum fields required are
- `"title"`
- `"artist"`
- `"diff"` and child fields, like `"drums"`, `"bass"`, `"guitar"`, and/or `"vocals"
- `"stems"` and at least its child field, `"backing"`
- `"midi"`
- `"sid"` or `"icon_drums"`
- `"sib"` or `"icon_bass"`
- `"sig"` or `"icon_guitar"`
- `"siv"` or `"icon_vocals"`

Because the info.json file uses JSON, it is very much noted that you should always follow JSON syntax. JSON and its various syntactial quirks will not be covered here, but the most notable issue is that capitalization is parsed, and any incorrect capitalization can render a field useless. 

Unlike other song formats, the midi file does not have to be named `notes.mid`, as long as its name is properly noted under the `"midi"` field.

## notes.mid

This is formatted extremely closely to a regular CH/RB chart that utilizes a `notes.mid`, although any plastic/5-fret/Pro/Classic chart should be in a track named `PLASTIC_<inst>`. All `PART_<inst>` tracks should be formatted for Encore pad gameplay.

*Note, `<inst>` is a placeholder for the instrument being charted. All acceptable instrument track names at this time are `PART_DRUMS`, `PART_BASS`, `PART_GUITAR`,`PART_VOCALS`, `PLASTIC_DRUMS`, `PLASTIC_BASS`, and `PLASTIC_GUITAR`.*

Refer to the chart below for the different difficulties and pitch information.

### Tap Lanes
|Difficulty|1|2|3|4|5|
|:-|:-:|:-:|:-:|:-:|:-:|
|Expert|96|97|98|99|100|
|Hard|84|85|86|87| |
|Medium|72|73|74|75| |
|Easy|60|61|62|63| |

### Lift Markers
|Difficulty|1|2|3|4|5|
|:-|:-:|:-:|:-:|:-:|:-:|
|Expert|102|103|104|105|106|
|Hard|90|91|92|93| |
|Medium|78|79|80|81| |
|Easy|66|67|68|69| |

### Overdrive

|Difficulty|Lane|
|:-|:-:|
|All Diffs|116|

## `EVENTS` and `BEAT`
Every `notes.mid` needs an `EVENTS` track and a `BEAT` track.

### Beat
`BEAT` is a mapping of each beat line to be displayed in Encore. On pitch 12 is any major beats, like the first beat of the measure. Even though you can technically use any other pitch, it is highly suggested to use pitch 13 for any other minor beat. Typically, the templates distributed will already do this for you.

### Events
`EVENTS` has three required text events. 
- A `[music_start]` event tells the game where the first beatline should appear. 
- A `[music_end]` event tells the game where to stop displaying beatlines. 
- An `[end]` event should be placed at the very end of the chart MIDI, or where the game should cut to the Results screen.

## Audio
All songs should be .ogg or .mp3, and include at least a backing/song track. 
There is no support for .flac or .opus. Ogg Opus support is a planned feature.

## Album art
Album art must be .png or .jpg, with a size of 512x512.