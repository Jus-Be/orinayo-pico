.\ffmpeg -y -i IN/1A/MAJ_C.wav -i IN/1B/MAJ_C.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/001_1/MAJ_C.wav
.\ffmpeg -y -i IN/1A/MIN_D.wav -i IN/1B/MIN_D.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/001_2/MIN_D.wav
.\ffmpeg -y -i IN/1A/MIN_E.wav -i IN/1B/MIN_E.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/001_3/MIN_E.wav
.\ffmpeg -y -i IN/1A/MAJ_F.wav -i IN/1B/MAJ_F.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/001_4/MAJ_F.wav
.\ffmpeg -y -i IN/1A/MAJ_G.wav -i IN/1B/MAJ_G.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/001_5/MAJ_G.wav
.\ffmpeg -y -i IN/1A/MIN_A.wav -i IN/1B/MIN_A.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/001_6/MIN_A.wav

.\ffmpeg -y -i IN/1A/MAJ_C#.wav -i IN/1B/MAJ_C#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/002_1/MAJ_C#.wav
.\ffmpeg -y -i IN/1A/MIN_D#.wav -i IN/1B/MIN_D#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/002_2/MIN_D#.wav
.\ffmpeg -y -i IN/1A/MIN_F.wav -i IN/1B/MIN_F.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/002_3/MIN_F.wav
.\ffmpeg -y -i IN/1A/MAJ_F#.wav -i IN/1B/MAJ_F#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/002_4/MAJ_F#.wav
.\ffmpeg -y -i IN/1A/MAJ_G#.wav -i IN/1B/MAJ_G#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/002_5/MAJ_G#.wav
.\ffmpeg -y -i IN/1A/MIN_A#.wav -i IN/1B/MIN_A#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/002_6/MIN_A#.wav

.\ffmpeg -y -i IN/1A/MAJ_D.wav -i IN/1B/MAJ_D.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/003_1/MAJ_D.wav
.\ffmpeg -y -i IN/1A/MIN_E.wav -i IN/1B/MIN_E.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/003_2/MIN_E.wav
.\ffmpeg -y -i IN/1A/MIN_F#.wav -i IN/1B/MIN_F#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/003_3/MIN_F#.wav
.\ffmpeg -y -i IN/1A/MAJ_G.wav -i IN/1B/MAJ_G.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/003_4/MAJ_G.wav
.\ffmpeg -y -i IN/1A/MAJ_A.wav -i IN/1B/MAJ_A.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/003_5/MAJ_A.wav
.\ffmpeg -y -i IN/1A/MIN_B.wav -i IN/1B/MIN_B.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/003_6/MIN_B.wav

.\ffmpeg -y -i IN/1A/MAJ_D#.wav -i IN/1B/MAJ_D#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/004_1/MAJ_D#.wav
.\ffmpeg -y -i IN/1A/MIN_F.wav -i IN/1B/MIN_F.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/004_2/MIN_F.wav
.\ffmpeg -y -i IN/1A/MIN_G.wav -i IN/1B/MIN_G.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/004_3/MIN_G.wav
.\ffmpeg -y -i IN/1A/MAJ_G#.wav -i IN/1B/MAJ_G#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/004_4/MAJ_G#.wav
.\ffmpeg -y -i IN/1A/MAJ_A#.wav -i IN/1B/MAJ_A#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/004_5/MAJ_A#.wav
.\ffmpeg -y -i IN/1A/MIN_C.wav -i IN/1B/MIN_C.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/004_6/MIN_C.wav

.\ffmpeg -y -i IN/1A/MAJ_E.wav -i IN/1B/MAJ_E.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/005_1/MAJ_E.wav
.\ffmpeg -y -i IN/1A/MIN_F#.wav -i IN/1B/MIN_F#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/005_2/MIN_F#.wav
.\ffmpeg -y -i IN/1A/MIN_G#.wav -i IN/1B/MIN_G#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/005_3/MIN_G#.wav
.\ffmpeg -y -i IN/1A/MAJ_A.wav -i IN/1B/MAJ_A.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/005_4/MAJ_A.wav
.\ffmpeg -y -i IN/1A/MAJ_B.wav -i IN/1B/MAJ_B.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/005_5/MAJ_B.wav
.\ffmpeg -y -i IN/1A/MIN_C#.wav -i IN/1B/MIN_C#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/005_6/MIN_C#.wav

.\ffmpeg -y -i IN/1A/MAJ_F.wav -i IN/1B/MAJ_F.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/006_1/MAJ_F.wav
.\ffmpeg -y -i IN/1A/MIN_G.wav -i IN/1B/MIN_G.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/006_2/MIN_G.wav
.\ffmpeg -y -i IN/1A/MIN_A.wav -i IN/1B/MIN_A.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/006_3/MIN_A.wav
.\ffmpeg -y -i IN/1A/MAJ_A#.wav -i IN/1B/MAJ_A#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/006_4/MAJ_A#.wav
.\ffmpeg -y -i IN/1A/MAJ_C.wav -i IN/1B/MAJ_C.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/006_5/MAJ_C.wav
.\ffmpeg -y -i IN/1A/MIN_D.wav -i IN/1B/MIN_D.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/006_6/MIN_D.wav

.\ffmpeg -y -i IN/1A/MAJ_F#.wav -i IN/1B/MAJ_F#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/007_1/MAJ_F#.wav
.\ffmpeg -y -i IN/1A/MIN_G#.wav -i IN/1B/MIN_G#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/007_2/MIN_G#.wav
.\ffmpeg -y -i IN/1A/MIN_A#.wav -i IN/1B/MIN_A#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/007_3/MIN_A#.wav
.\ffmpeg -y -i IN/1A/MAJ_B.wav -i IN/1B/MAJ_B.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/007_4/MAJ_B.wav
.\ffmpeg -y -i IN/1A/MAJ_C#.wav -i IN/1B/MAJ_C#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/007_5/MAJ_C#.wav
.\ffmpeg -y -i IN/1A/MIN_D#.wav -i IN/1B/MIN_D#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/007_6/MIN_D#.wav

.\ffmpeg -y -i IN/1A/MAJ_G.wav -i IN/1B/MAJ_G.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/008_1/MAJ_G.wav
.\ffmpeg -y -i IN/1A/MIN_A.wav -i IN/1B/MIN_A.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/008_2/MIN_A.wav
.\ffmpeg -y -i IN/1A/MIN_B.wav -i IN/1B/MIN_B.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/008_3/MIN_B.wav
.\ffmpeg -y -i IN/1A/MAJ_C.wav -i IN/1B/MAJ_C.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/008_4/MAJ_C.wav
.\ffmpeg -y -i IN/1A/MAJ_D.wav -i IN/1B/MAJ_D.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/008_5/MAJ_D.wav
.\ffmpeg -y -i IN/1A/MIN_E.wav -i IN/1B/MIN_E.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/008_6/MIN_E.wav

.\ffmpeg -y -i IN/1A/MAJ_G#.wav -i IN/1B/MAJ_G#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/009_1/MAJ_G#.wav
.\ffmpeg -y -i IN/1A/MIN_A#.wav -i IN/1B/MIN_A#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/009_2/MIN_A#.wav
.\ffmpeg -y -i IN/1A/MIN_C.wav -i IN/1B/MIN_C.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/009_3/MIN_C.wav
.\ffmpeg -y -i IN/1A/MAJ_C#.wav -i IN/1B/MAJ_C#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/009_4/MAJ_C#.wav
.\ffmpeg -y -i IN/1A/MAJ_D#.wav -i IN/1B/MAJ_D#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/009_5/MAJ_D#.wav
.\ffmpeg -y -i IN/1A/MIN_F.wav -i IN/1B/MIN_F.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/009_6/MIN_F.wav

.\ffmpeg -y -i IN/1A/MAJ_A.wav -i IN/1B/MAJ_A.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/010_1/MAJ_A.wav
.\ffmpeg -y -i IN/1A/MIN_B.wav -i IN/1B/MIN_B.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/010_2/MIN_B.wav
.\ffmpeg -y -i IN/1A/MIN_C#.wav -i IN/1B/MIN_C#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/010_3/MIN_C#.wav
.\ffmpeg -y -i IN/1A/MAJ_D.wav -i IN/1B/MAJ_D.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/010_4/MAJ_D.wav
.\ffmpeg -y -i IN/1A/MAJ_E.wav -i IN/1B/MAJ_E.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/010_5/MAJ_E.wav
.\ffmpeg -y -i IN/1A/MIN_F#.wav -i IN/1B/MIN_F#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/010_6/MIN_F#.wav

.\ffmpeg -y -i IN/1A/MAJ_A#.wav -i IN/1B/MAJ_A#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/011_1/MAJ_A#.wav
.\ffmpeg -y -i IN/1A/MIN_C.wav -i IN/1B/MIN_C.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/011_2/MIN_C.wav
.\ffmpeg -y -i IN/1A/MIN_D.wav -i IN/1B/MIN_D.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/011_3/MIN_D.wav
.\ffmpeg -y -i IN/1A/MAJ_D#.wav -i IN/1B/MAJ_D#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/011_4/MAJ_D#.wav
.\ffmpeg -y -i IN/1A/MAJ_F.wav -i IN/1B/MAJ_F.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/011_5/MAJ_F.wav
.\ffmpeg -y -i IN/1A/MIN_G.wav -i IN/1B/MIN_G.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/011_6/MIN_G.wav

.\ffmpeg -y -i IN/1A/MAJ_B.wav -i IN/1B/MAJ_B.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/012_1/MAJ_B.wav
.\ffmpeg -y -i IN/1A/MIN_C#.wav -i IN/1B/MIN_C#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/012_2/MIN_C#.wav
.\ffmpeg -y -i IN/1A/MIN_D#.wav -i IN/1B/MIN_D#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/012_3/MIN_D#.wav
.\ffmpeg -y -i IN/1A/MAJ_E.wav -i IN/1B/MAJ_E.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/012_4/MAJ_E.wav
.\ffmpeg -y -i IN/1A/MAJ_F#.wav -i IN/1B/MAJ_F#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/012_5/MAJ_F#.wav
.\ffmpeg -y -i IN/1A/MIN_G#.wav -i IN/1B/MIN_G#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/012_6/MIN_G#.wav

