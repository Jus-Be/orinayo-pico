.\ffmpeg -y -i IN/2A/MAJ_C.wav -i IN/2B/MAJ_C.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/013_1/MAJ_C.wav
.\ffmpeg -y -i IN/2A/MIN_D.wav -i IN/2B/MIN_D.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/013_2/MIN_D.wav
.\ffmpeg -y -i IN/2A/MIN_E.wav -i IN/2B/MIN_E.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/013_3/MIN_E.wav
.\ffmpeg -y -i IN/2A/MAJ_F.wav -i IN/2B/MAJ_F.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/013_4/MAJ_F.wav
.\ffmpeg -y -i IN/2A/MAJ_G.wav -i IN/2B/MAJ_G.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/013_5/MAJ_G.wav
.\ffmpeg -y -i IN/2A/MIN_A.wav -i IN/2B/MIN_A.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/013_6/MIN_A.wav

.\ffmpeg -y -i IN/2A/MAJ_C#.wav -i IN/2B/MAJ_C#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/014_1/MAJ_C#.wav
.\ffmpeg -y -i IN/2A/MIN_D#.wav -i IN/2B/MIN_D#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/014_2/MIN_D#.wav
.\ffmpeg -y -i IN/2A/MIN_F.wav -i IN/2B/MIN_F.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/014_3/MIN_F.wav
.\ffmpeg -y -i IN/2A/MAJ_F#.wav -i IN/2B/MAJ_F#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/014_4/MAJ_F#.wav
.\ffmpeg -y -i IN/2A/MAJ_G#.wav -i IN/2B/MAJ_G#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/014_5/MAJ_G#.wav
.\ffmpeg -y -i IN/2A/MIN_A#.wav -i IN/2B/MIN_A#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/014_6/MIN_A#.wav

.\ffmpeg -y -i IN/2A/MAJ_D.wav -i IN/2B/MAJ_D.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/015_1/MAJ_D.wav
.\ffmpeg -y -i IN/2A/MIN_E.wav -i IN/2B/MIN_E.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/015_2/MIN_E.wav
.\ffmpeg -y -i IN/2A/MIN_F#.wav -i IN/2B/MIN_F#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/015_3/MIN_F#.wav
.\ffmpeg -y -i IN/2A/MAJ_G.wav -i IN/2B/MAJ_G.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/015_4/MAJ_G.wav
.\ffmpeg -y -i IN/2A/MAJ_A.wav -i IN/2B/MAJ_A.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/015_5/MAJ_A.wav
.\ffmpeg -y -i IN/2A/MIN_B.wav -i IN/2B/MIN_B.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/015_6/MIN_B.wav

.\ffmpeg -y -i IN/2A/MAJ_D#.wav -i IN/2B/MAJ_D#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/016_1/MAJ_D#.wav
.\ffmpeg -y -i IN/2A/MIN_F.wav -i IN/2B/MIN_F.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/016_2/MIN_F.wav
.\ffmpeg -y -i IN/2A/MIN_G.wav -i IN/2B/MIN_G.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/016_3/MIN_G.wav
.\ffmpeg -y -i IN/2A/MAJ_G#.wav -i IN/2B/MAJ_G#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/016_4/MAJ_G#.wav
.\ffmpeg -y -i IN/2A/MAJ_A#.wav -i IN/2B/MAJ_A#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/016_5/MAJ_A#.wav
.\ffmpeg -y -i IN/2A/MIN_C.wav -i IN/2B/MIN_C.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/016_6/MIN_C.wav

.\ffmpeg -y -i IN/2A/MAJ_E.wav -i IN/2B/MAJ_E.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/017_1/MAJ_E.wav
.\ffmpeg -y -i IN/2A/MIN_F#.wav -i IN/2B/MIN_F#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/017_2/MIN_F#.wav
.\ffmpeg -y -i IN/2A/MIN_G#.wav -i IN/2B/MIN_G#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/017_3/MIN_G#.wav
.\ffmpeg -y -i IN/2A/MAJ_A.wav -i IN/2B/MAJ_A.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/017_4/MAJ_A.wav
.\ffmpeg -y -i IN/2A/MAJ_B.wav -i IN/2B/MAJ_B.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/017_5/MAJ_B.wav
.\ffmpeg -y -i IN/2A/MIN_C#.wav -i IN/2B/MIN_C#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/017_6/MIN_C#.wav

.\ffmpeg -y -i IN/2A/MAJ_F.wav -i IN/2B/MAJ_F.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/018_1/MAJ_F.wav
.\ffmpeg -y -i IN/2A/MIN_G.wav -i IN/2B/MIN_G.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/018_2/MIN_G.wav
.\ffmpeg -y -i IN/2A/MIN_A.wav -i IN/2B/MIN_A.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/018_3/MIN_A.wav
.\ffmpeg -y -i IN/2A/MAJ_A#.wav -i IN/2B/MAJ_A#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/018_4/MAJ_A#.wav
.\ffmpeg -y -i IN/2A/MAJ_C.wav -i IN/2B/MAJ_C.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/018_5/MAJ_C.wav
.\ffmpeg -y -i IN/2A/MIN_D.wav -i IN/2B/MIN_D.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/018_6/MIN_D.wav

.\ffmpeg -y -i IN/2A/MAJ_F#.wav -i IN/2B/MAJ_F#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/019_1/MAJ_F#.wav
.\ffmpeg -y -i IN/2A/MIN_G#.wav -i IN/2B/MIN_G#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/019_2/MIN_G#.wav
.\ffmpeg -y -i IN/2A/MIN_A#.wav -i IN/2B/MIN_A#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/019_3/MIN_A#.wav
.\ffmpeg -y -i IN/2A/MAJ_B.wav -i IN/2B/MAJ_B.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/019_4/MAJ_B.wav
.\ffmpeg -y -i IN/2A/MAJ_C#.wav -i IN/2B/MAJ_C#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/019_5/MAJ_C#.wav
.\ffmpeg -y -i IN/2A/MIN_D#.wav -i IN/2B/MIN_D#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/019_6/MIN_D#.wav

.\ffmpeg -y -i IN/2A/MAJ_G.wav -i IN/2B/MAJ_G.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/020_1/MAJ_G.wav
.\ffmpeg -y -i IN/2A/MIN_A.wav -i IN/2B/MIN_A.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/020_2/MIN_A.wav
.\ffmpeg -y -i IN/2A/MIN_B.wav -i IN/2B/MIN_B.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/020_3/MIN_B.wav
.\ffmpeg -y -i IN/2A/MAJ_C.wav -i IN/2B/MAJ_C.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/020_4/MAJ_C.wav
.\ffmpeg -y -i IN/2A/MAJ_D.wav -i IN/2B/MAJ_D.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/020_5/MAJ_D.wav
.\ffmpeg -y -i IN/2A/MIN_E.wav -i IN/2B/MIN_E.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/020_6/MIN_E.wav

.\ffmpeg -y -i IN/2A/MAJ_G#.wav -i IN/2B/MAJ_G#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/021_1/MAJ_G#.wav
.\ffmpeg -y -i IN/2A/MIN_A#.wav -i IN/2B/MIN_A#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/021_2/MIN_A#.wav
.\ffmpeg -y -i IN/2A/MIN_C.wav -i IN/2B/MIN_C.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/021_3/MIN_C.wav
.\ffmpeg -y -i IN/2A/MAJ_C#.wav -i IN/2B/MAJ_C#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/021_4/MAJ_C#.wav
.\ffmpeg -y -i IN/2A/MAJ_D#.wav -i IN/2B/MAJ_D#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/021_5/MAJ_D#.wav
.\ffmpeg -y -i IN/2A/MIN_F.wav -i IN/2B/MIN_F.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/021_6/MIN_F.wav

.\ffmpeg -y -i IN/2A/MAJ_A.wav -i IN/2B/MAJ_A.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/022_1/MAJ_A.wav
.\ffmpeg -y -i IN/2A/MIN_B.wav -i IN/2B/MIN_B.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/022_2/MIN_B.wav
.\ffmpeg -y -i IN/2A/MIN_C#.wav -i IN/2B/MIN_C#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/022_3/MIN_C#.wav
.\ffmpeg -y -i IN/2A/MAJ_D.wav -i IN/2B/MAJ_D.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/022_4/MAJ_D.wav
.\ffmpeg -y -i IN/2A/MAJ_E.wav -i IN/2B/MAJ_E.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/022_5/MAJ_E.wav
.\ffmpeg -y -i IN/2A/MIN_F#.wav -i IN/2B/MIN_F#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/022_6/MIN_F#.wav

.\ffmpeg -y -i IN/2A/MAJ_A#.wav -i IN/2B/MAJ_A#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/023_1/MAJ_A#.wav
.\ffmpeg -y -i IN/2A/MIN_C.wav -i IN/2B/MIN_C.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/023_2/MIN_C.wav
.\ffmpeg -y -i IN/2A/MIN_D.wav -i IN/2B/MIN_D.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/023_3/MIN_D.wav
.\ffmpeg -y -i IN/2A/MAJ_D#.wav -i IN/2B/MAJ_D#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/023_4/MAJ_D#.wav
.\ffmpeg -y -i IN/2A/MAJ_F.wav -i IN/2B/MAJ_F.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/023_5/MAJ_F.wav
.\ffmpeg -y -i IN/2A/MIN_G.wav -i IN/2B/MIN_G.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/023_6/MIN_G.wav

.\ffmpeg -y -i IN/2A/MAJ_B.wav -i IN/2B/MAJ_B.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/024_1/MAJ_B.wav
.\ffmpeg -y -i IN/2A/MIN_C#.wav -i IN/2B/MIN_C#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/024_2/MIN_C#.wav
.\ffmpeg -y -i IN/2A/MIN_D#.wav -i IN/2B/MIN_D#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/024_3/MIN_D#.wav
.\ffmpeg -y -i IN/2A/MAJ_E.wav -i IN/2B/MAJ_E.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/024_4/MAJ_E.wav
.\ffmpeg -y -i IN/2A/MAJ_F#.wav -i IN/2B/MAJ_F#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/024_5/MAJ_F#.wav
.\ffmpeg -y -i IN/2A/MIN_G#.wav -i IN/2B/MIN_G#.wav -filter_complex "[0:0][1:0]concat=n=2:v=0:a=1[out]" -map "[out]"  WAVE/024_6/MIN_G#.wav

