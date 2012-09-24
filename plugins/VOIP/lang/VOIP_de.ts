<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.0" language="de_DE">
<context>
    <name>AudioInput</name>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="14"/>
        <source>Form</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="20"/>
        <source>Audio Wizard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="33"/>
        <source>Transmission</source>
        <translation>Übertragung</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="39"/>
        <source>&amp;Transmit</source>
        <translation>Über&amp;tragen</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="49"/>
        <source>When to transmit your speech</source>
        <translation>Wann Sprache übertragen werden soll</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="52"/>
        <source>&lt;b&gt;This sets when speech should be transmitted.&lt;/b&gt;&lt;br /&gt;&lt;i&gt;Continuous&lt;/i&gt; - All the time&lt;br /&gt;&lt;i&gt;Voice Activity&lt;/i&gt; - When you are speaking clearly.&lt;br /&gt;&lt;i&gt;Push To Talk&lt;/i&gt; - When you hold down the hotkey set under &lt;i&gt;Shortcuts&lt;/i&gt;.</source>
        <translation>&lt;b&gt;Dies legt fest, wann Sprache übertragen werden soll.&lt;/b&gt;&lt;br /&gt;&lt;i&gt;Kontinuierlich&lt;/i&gt; - Die ganze Zeit&lt;br /&gt;&lt;i&gt;Stimmaktivierung&lt;/i&gt; - Sobald man deutlich spricht.&lt;/br&gt;&lt;i&gt;Push-To-Talk&lt;/i&gt; - Wenn ein Hotkey gedrückt wird (siehe &lt;i&gt;Shortcuts&lt;/i&gt;).</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="66"/>
        <source>DoublePush Time</source>
        <translation>Doppeldruck-Zeit</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="76"/>
        <source>If you press the PTT key twice in this time it will get locked.</source>
        <translation>Wenn Sie die PTT-Taste (Sprech-Taste) zweimal innerhalb dieser Zeit drücken wird die Sprachübertragung dauerhaft aktiviert.</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="79"/>
        <source>&lt;b&gt;DoublePush Time&lt;/b&gt;&lt;br /&gt;If you press the push-to-talk key twice during the configured interval of time it will be locked. Mumble will keep transmitting until you hit the key once more to unlock PTT again.</source>
        <translation>&lt;b&gt;Doppeldruck-Zeit&lt;/b&gt;&lt;br /&gt;Wenn Sie die PTT-Taste zweimal innerhalb der Doppeldruck-Zeit drücken wird die Sprachübertragung dauerhaft aktiviert. Diese wird beendet wenn sie die Taste ein weiteres mal drücken.</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="122"/>
        <source>Voice &amp;Hold</source>
        <translation>Stimme &amp;halten</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="132"/>
        <source>How long to keep transmitting after silence</source>
        <translation>Wie lange nach dem Einsetzen von Stille übertragen werden soll</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="135"/>
        <source>&lt;b&gt;This selects how long after a perceived stop in speech transmission should continue.&lt;/b&gt;&lt;br /&gt;Set this higher if your voice breaks up when you speak (seen by a rapidly blinking voice icon next to your name).</source>
        <translation>&lt;b&gt;Hiermit bestimmen Sie, wie lange nach Beenden des Gesprächs noch übertragen werden soll.&lt;/b&gt;&lt;br /&gt;Höhere Werte sind hilfreich, wenn die Stimme plötzlich abbricht (erkennbar an einem flackerndem Voice-Icon neben dem Namen).</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="151"/>
        <source>Silence Below</source>
        <translation>Stille bis</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="158"/>
        <source>Signal values below this count as silence</source>
        <translation>Signalwerte darunter zählen als Stille</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="161"/>
        <location filename="../gui/AudioInputConfig.ui" line="193"/>
        <source>&lt;b&gt;This sets the trigger values for voice detection.&lt;/b&gt;&lt;br /&gt;Use this together with the Audio Statistics window to manually tune the trigger values for detecting speech. Input values below &quot;Silence Below&quot; always count as silence. Values above &quot;Speech Above&quot; always count as voice. Values in between will count as voice if you&apos;re already talking, but will not trigger a new detection.</source>
        <translation>&lt;b&gt;Dies setzt die Auslösewerte für die Spracherkennung.&lt;/b&gt;&lt;br /&gt;Zusammen mit dem Audiostatistik Fenster können die Auslösewerte für die Spracherkennung manuell eingestellt werden. Eingabewerte unter &quot;Stille bis&quot; zählen immer als Stille, Werte über &quot;Sprache über&quot; immer als Sprache. Werte dazwischen zählen als Sprache wenn schon gesprochen wird, lösen aber keine Erkennung (und damit Übertragung) aus.</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="183"/>
        <source>Speech Above</source>
        <translation>Sprache über</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="190"/>
        <source>Signal values above this count as voice</source>
        <translation>Signalwerte darüber zählen als Sprache</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="228"/>
        <source>empty</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="243"/>
        <source>Audio Processing</source>
        <translation>Audioverarbeitung</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="249"/>
        <source>Noise Suppression</source>
        <translation>Rauschunterdrückung</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="262"/>
        <source>Noise suppression</source>
        <translation>Rauschunterdrückung</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="265"/>
        <source>&lt;b&gt;This sets the amount of noise suppression to apply.&lt;/b&gt;&lt;br /&gt;The higher this value, the more aggressively stationary noise will be suppressed.</source>
        <translation>&lt;b&gt;Dies setzt die Stärke der Rauschunterdrückung die angewandt werden soll&lt;/b&gt;&lt;br /&gt;Je höher der Wert, desto aggressiver wird Rauschen unterdrückt.</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="297"/>
        <source>Amplification</source>
        <translation>Verstärkung</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="307"/>
        <source>Maximum amplification of input sound</source>
        <translation>Maximale Verstärkung des Eingangssignals</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="310"/>
        <source>&lt;b&gt;Maximum amplification of input.&lt;/b&gt;&lt;br /&gt;Mumble normalizes the input volume before compressing, and this sets how much it&apos;s allowed to amplify.&lt;br /&gt;The actual level is continually updated based on your current speech pattern, but it will never go above the level specified here.&lt;br /&gt;If the &lt;i&gt;Microphone loudness&lt;/i&gt; level of the audio statistics hover around 100%, you probably want to set this to 2.0 or so, but if, like most people, you are unable to reach 100%, set this to something much higher.&lt;br /&gt;Ideally, set it so &lt;i&gt;Microphone Loudness * Amplification Factor &gt;= 100&lt;/i&gt;, even when you&apos;re speaking really soft.&lt;br /&gt;&lt;br /&gt;Note that there is no harm in setting this to maximum, but Mumble will start picking up other conversations if you leave it to auto-tune to that level.</source>
        <translation>&lt;b&gt;Maximale Verstärkung des Eingangssignals.&lt;/b&gt;&lt;br /&gt;RetroShare normalisiert die Eingangslautstärke vor der Kompression, wobei diese Option festlegt wie sehr verstärkt werden darf.&lt;br /&gt;Der tatsächliche Level wird kontinuierlich, abhängig vom Sprachmuster, aktualisiert; allerdings nie höher als hier festgelegt.&lt;br /&gt;Wenn die Mikrofonlautstärke in den Audiostatistiken um 100% liegt, sollte man dies auf 2.0 setzen. Für Leute die dies kaum erreichen, muss es deutlich höher angesetzt werden.&lt;br /&gt;Idealerweise sollte es folgendermaßen gesetzt werden: &lt;i&gt;Mikrofon Lautstärke * Verstärkungsfaktor &gt;= 100&lt;/i&gt;, selbst wenn man wirklich leise spricht.&lt;br /&gt;Es ist nicht schädlich dies auf das Maximum zu setzen, aber Mumble wird dadurch auch Umgebungsgeräusche aufnehmen.</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.ui" line="342"/>
        <source>Echo Cancellation Processing</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>AudioInputConfig</name>
    <message>
        <location filename="../gui/AudioInputConfig.cpp" line="98"/>
        <source>Continuous</source>
        <translation>Kontinuierlich</translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.cpp" line="99"/>
        <source>Voice Activity</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.cpp" line="100"/>
        <source>Push To Talk</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.cpp" line="202"/>
        <source>%1 s</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.cpp" line="210"/>
        <source>Off</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.cpp" line="213"/>
        <source>-%1 dB</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioInputConfig.h" line="72"/>
        <source>VOIP</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>AudioStats</name>
    <message>
        <location filename="../gui/AudioStats.ui" line="14"/>
        <source>Audio Statistics</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="22"/>
        <source>Input Levels</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="28"/>
        <source>Peak microphone level</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="35"/>
        <location filename="../gui/AudioStats.ui" line="55"/>
        <location filename="../gui/AudioStats.ui" line="75"/>
        <source>Peak power in last frame</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="38"/>
        <source>This shows the peak power in the last frame (20 ms), and is the same measurement as you would usually find displayed as &quot;input power&quot;. Please disregard this and look at &lt;b&gt;Microphone power&lt;/b&gt; instead, which is much more steady and disregards outliers.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="48"/>
        <source>Peak speaker level</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="58"/>
        <source>This shows the peak power of the speakers in the last frame (20 ms). Unless you are using a multi-channel sampling method (such as ASIO) with speaker channels configured, this will be 0. If you have such a setup configured, and this still shows 0 while you&apos;re playing audio from other programs, your setup is not working.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="68"/>
        <source>Peak clean level</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="78"/>
        <source>This shows the peak power in the last frame (20 ms) after all processing. Ideally, this should be -96 dB when you&apos;re not talking. In reality, a sound studio should see -60 dB, and you should hopefully see somewhere around -20 dB. When you are talking, this should rise to somewhere between -5 and -10 dB.&lt;br /&gt;If you are using echo cancellation, and this rises to more than -15 dB when you&apos;re not talking, your setup is not working, and you&apos;ll annoy other users with echoes.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="91"/>
        <source>Signal Analysis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="97"/>
        <source>Microphone power</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="104"/>
        <source>How close the current input level is to ideal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="107"/>
        <source>This shows how close your current input volume is to the ideal. To adjust your microphone level, open whatever program you use to adjust the recording volume, and look at the value here while talking.&lt;br /&gt;&lt;b&gt;Talk loud, as you would when you&apos;re upset over getting fragged by a noob.&lt;/b&gt;&lt;br /&gt;Adjust the volume until this value is close to 100%, but make sure it doesn&apos;t go above. If it does go above, you are likely to get clipping in parts of your speech, which will degrade sound quality.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="117"/>
        <source>Signal-To-Noise ratio</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="124"/>
        <source>Signal-To-Noise ratio from the microphone</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="127"/>
        <source>This is the Signal-To-Noise Ratio (SNR) of the microphone in the last frame (20 ms). It shows how much clearer the voice is compared to the noise.&lt;br /&gt;If this value is below 1.0, there&apos;s more noise than voice in the signal, and so quality is reduced.&lt;br /&gt;There is no upper limit to this value, but don&apos;t expect to see much above 40-50 without a sound studio.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="137"/>
        <source>Speech Probability</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="144"/>
        <source>Probability of speech</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="147"/>
        <source>This is the probability that the last frame (20 ms) was speech and not environment noise.&lt;br /&gt;Voice activity transmission depends on this being right. The trick with this is that the middle of a sentence is always detected as speech; the problem is the pauses between words and the start of speech. It&apos;s hard to distinguish a sigh from a word starting with &apos;h&apos;.&lt;br /&gt;If this is in bold font, it means Mumble is currently transmitting (if you&apos;re connected).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="162"/>
        <source>Configuration feedback</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="168"/>
        <source>Current audio bitrate</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="181"/>
        <source>Bitrate of last frame</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="184"/>
        <source>This is the audio bitrate of the last compressed frame (20 ms), and as such will jump up and down as the VBR adjusts the quality. The peak bitrate can be adjusted in the Settings dialog.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="194"/>
        <source>DoublePush interval</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="207"/>
        <source>Time between last two Push-To-Talk presses</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="217"/>
        <source>Speech Detection</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="224"/>
        <source>Current speech detection chance</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="227"/>
        <source>&lt;b&gt;This shows the current speech detection settings.&lt;/b&gt;&lt;br /&gt;You can change the settings from the Settings dialog or from the Audio Wizard.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="256"/>
        <source>Signal and noise power spectrum</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="262"/>
        <source>Power spectrum of input signal and noise estimate</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="265"/>
        <source>This shows the power spectrum of the current input signal (red line) and the current noise estimate (filled blue).&lt;br /&gt;All amplitudes are multiplied by 30 to show the interesting parts (how much more signal than noise is present in each waveband).&lt;br /&gt;This is probably only of interest if you&apos;re trying to fine-tune noise conditions on your microphone. Under good conditions, there should be just a tiny flutter of blue at the bottom. If the blue is more than halfway up on the graph, you have a seriously noisy environment.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="281"/>
        <source>Echo Analysis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="293"/>
        <source>Weights of the echo canceller</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioStats.ui" line="296"/>
        <source>This shows the weights of the echo canceller, with time increasing downwards and frequency increasing to the right.&lt;br /&gt;Ideally, this should be black, indicating no echo exists at all. More commonly, you&apos;ll have one or more horizontal stripes of bluish color representing time delayed echo. You should be able to see the weights updated in real time.&lt;br /&gt;Please note that as long as you have nothing to echo off, you won&apos;t see much useful data here. Play some music and things should stabilize. &lt;br /&gt;You can choose to view the real or imaginary parts of the frequency-domain weights, or alternately the computed modulus and phase. The most useful of these will likely be modulus, which is the amplitude of the echo, and shows you how much of the outgoing signal is being removed at that time step. The other viewing modes are mostly useful to people who want to tune the echo cancellation algorithms.&lt;br /&gt;Please note: If the entire image fluctuates massively while in modulus mode, the echo canceller fails to find any correlation whatsoever between the two input sources (speakers and microphone). Either you have a very long delay on the echo, or one of the input sources is configured wrong.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>AudioWizard</name>
    <message>
        <location filename="../gui/AudioWizard.ui" line="14"/>
        <source>Audio Tuning Wizard</source>
        <translation>Audio Einstellungs-Assistent</translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="18"/>
        <source>Introduction</source>
        <translation>Einführung</translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="21"/>
        <source>Welcome to the RetroShare Audio Wizard</source>
        <translation>Willkommen zum RetroShare Audio-Assistenten</translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="27"/>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;MS Shell Dlg 2&apos;; font-size:8.25pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;&lt;span style=&quot; font-family:&apos;Cantarell&apos;; font-size:11pt;&quot;&gt;This is the audio tuning wizard for RetroShare. This will help you correctly set the input levels of your sound card, and also set the correct parameters for sound processing in Retroshare. &lt;/span&gt;&lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;p&gt;
Dies ist RetroShare Assistent zum konfigurieren Ihrer Audio-Einstellungen. Er wird Ihnen helfen die korrekte Eingangslautstärke Ihrer Soundkarte und die korrekten Parameter für die Tonverarbeitung in RetroShare zu wählen.
&lt;/p&gt;</translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="55"/>
        <source>Volume tuning</source>
        <translation>Lautstärken-Einstellung</translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="58"/>
        <source>Tuning microphone hardware volume to optimal settings.</source>
        <translation>Mikrofonhardware-Lautstärke auf optimalen Wert einstellen.</translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="64"/>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Cantarell&apos;; font-size:11pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Open your sound control panel and go to the recording settings. Make sure the microphone is selected as active input with maximum recording volume. If there&apos;s an option to enable a &amp;quot;Microphone boost&amp;quot; make sure it&apos;s checked. &lt;/p&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Speak loudly, as when you are annoyed or excited. Decrease the volume in the sound control panel until the bar below stays as high as possible in the green and orange but &lt;span style=&quot; font-weight:600;&quot;&gt;not&lt;/span&gt; the red zone while you speak. &lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>Öffnen Sie die Lautstärkeeinstellungen und gehen Sie zu den Aufnahmeeinstellungen. Versichern Sie sich, dass das Mikrofon als aktives Eingabegerät mit maximaler Aufnahmelautstärke ausgewählt ist. Falls es eine Option &quot;Mikrofon Boost&quot; gibt, sollte diese aktiviert sein.

Sprechen Sie so laut, als wären Sie verärgert oder aufgeregt. Verringern Sie die Lautstärke in den Lautstärkeeinstellungen bis der Balken so weit wie möglich oben im blauen und grünen, aber nicht im roten Bereich ist, während Sie sprechen.</translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="79"/>
        <source>Talk normally, and adjust the slider below so that the bar moves into green when you talk, and doesn&apos;t go into the orange zone.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="123"/>
        <source>Stop looping echo for this wizard</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="143"/>
        <source>Apply some high contrast optimizations for visually impaired users</source>
        <translation>Auf hohen Kontrast optimierte Darstellung für sehbehinderte Benutzer verwenden</translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="146"/>
        <source>Use high contrast graphics</source>
        <translation>Anzeigen mit hohem Kontrast verwenden</translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="156"/>
        <source>Voice Activity Detection</source>
        <translation>Sprachaktivitätserkennung</translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="159"/>
        <source>Letting RetroShare figure out when you&apos;re talking and when you&apos;re silent.</source>
        <translation>Lassen Sie RetroShare herausfinden wann Sie sprechen und wann nicht.</translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="165"/>
        <source>This will help Retroshare figure out when you are talking. The first step is selecting which data value to use.</source>
        <translation>Dies wird RetroShare helfen herauszufinden, wann Sie sprechen. Der erste Schritt ist den zu benutzenden Datenwert auszuwählen.</translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="177"/>
        <source>Push To Talk:</source>
        <translation>Push-To-Talk:</translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="184"/>
        <source>todo shortcut</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="206"/>
        <source>Voice Detection</source>
        <translation>Voice-Erkennung</translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="219"/>
        <source>Next you need to adjust the following slider. The first few utterances you say should end up in the green area (definitive speech). While talking, you should stay inside the yellow (might be speech) and when you&apos;re not talking, everything should be in the red (definitively not speech).</source>
        <translation>Als nächstes müssen Sie den folgenden Schieber anpassen. Die ersten paar Geräusche die Sie beim Sprechen machen sollten im grünen Bereich (definitv Sprache) landen. Während Sie sprechen sollten Sie im gelben Bereich (könnte Sprache sein) bleiben und wenn Sie nicht sprechen, sollte alles im roten Bereich (definitiv keine Sprache) bleiben.</translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="283"/>
        <source>Continuous transmission</source>
        <translation>Kontinuierliche Übertragung</translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="291"/>
        <source>Finished</source>
        <translation>Fertig</translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="294"/>
        <source>Enjoy using RetroShare</source>
        <translation>Viel Spaß mit RetroShare</translation>
    </message>
    <message>
        <location filename="../gui/AudioWizard.ui" line="300"/>
        <source>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Cantarell&apos;; font-size:11pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Congratulations. You should now be ready to enjoy a richer sound experience with Retroshare. &lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</source>
        <translation>&lt;!DOCTYPE HTML PUBLIC &quot;-//W3C//DTD HTML 4.0//EN&quot; &quot;http://www.w3.org/TR/REC-html40/strict.dtd&quot;&gt;
&lt;html&gt;&lt;head&gt;&lt;meta name=&quot;qrichtext&quot; content=&quot;1&quot; /&gt;&lt;style type=&quot;text/css&quot;&gt;
p, li { white-space: pre-wrap; }
&lt;/style&gt;&lt;/head&gt;&lt;body style=&quot; font-family:&apos;Cantarell&apos;; font-size:11pt; font-weight:400; font-style:normal;&quot;&gt;
&lt;p style=&quot; margin-top:12px; margin-bottom:12px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;&quot;&gt;Herzlichen Glückwunsch. Sie sollten nun eine reichere Sounderfahrung mit Retroshare machen. &lt;/p&gt;&lt;/body&gt;&lt;/html&gt;</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="../VOIPPlugin.cpp" line="92"/>
        <source>&lt;h3&gt;RetroShare VOIP plugin&lt;/h3&gt;&lt;br/&gt;   * Contributors: Cyril Soler, Josselin Jacquard&lt;br/&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../VOIPPlugin.cpp" line="93"/>
        <source>&lt;br/&gt;The VOIP plugin adds VOIP to the private chat window of RetroShare. to use it, proceed as follows:&lt;UL&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../VOIPPlugin.cpp" line="94"/>
        <source>&lt;li&gt; setup microphone levels using the configuration panel&lt;/li&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../VOIPPlugin.cpp" line="95"/>
        <source>&lt;li&gt; check your microphone by looking at the VU-metters&lt;/li&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../VOIPPlugin.cpp" line="96"/>
        <source>&lt;li&gt; in the private chat, enable sound input/output by clicking on the two VOIP icons&lt;/li&gt;&lt;/ul&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../VOIPPlugin.cpp" line="97"/>
        <source>Your friend needs to run the plugin to talk/listen to you, or course.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../VOIPPlugin.cpp" line="98"/>
        <source>&lt;br/&gt;&lt;br/&gt;This is an experimental feature. Don&apos;t hesitate to send comments and suggestion to the RS dev team.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../VOIPPlugin.cpp" line="116"/>
        <source>RTT Statistics</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/VoipStatistics.cpp" line="145"/>
        <location filename="../gui/VoipStatistics.cpp" line="147"/>
        <location filename="../gui/VoipStatistics.cpp" line="149"/>
        <source>secs</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/VoipStatistics.cpp" line="151"/>
        <source>Old</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../gui/VoipStatistics.cpp" line="152"/>
        <source>Now</source>
        <translation type="unfinished">Jetzt</translation>
    </message>
    <message>
        <location filename="../gui/VoipStatistics.cpp" line="361"/>
        <source>Round Trip Time:</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>VOIP</name>
    <message>
        <location filename="../VOIPPlugin.cpp" line="152"/>
        <source>This plugin provides voice communication between friends in RetroShare.</source>
        <translation>Dieses Plugin bietet Sprach-Kommunikation zwischen Freunden in RetroShare.</translation>
    </message>
</context>
<context>
    <name>VOIPPlugin</name>
    <message>
        <location filename="../VOIPPlugin.cpp" line="157"/>
        <source>VOIP</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>VoipStatistics</name>
    <message>
        <location filename="../gui/VoipStatistics.ui" line="14"/>
        <source>VoipTest Statistics</source>
        <translation>Voip Test Statistiken</translation>
    </message>
</context>
</TS>