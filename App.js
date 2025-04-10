import React, { useState, useEffect } from 'react';
import MapComponent from './components/MapComponent';
import { getDuraklar, getEnYakinDurak } from './services/api';

function App() {
    const [userLocation, setUserLocation] = useState(null);
    const [destination, setDestination] = useState(null);
    const [duraklar, setDuraklar] = useState([]);
    const [yolcuTipi, setYolcuTipi] = useState('Genel');
    const [odemeTipi, setOdemeTipi] = useState('Nakit');
    const [bakiye, setBakiye] = useState('');
    const [mesaj, setMesaj] = useState('');
    const [rotaKoordinatlari, setRotaKoordinatlari] = useState([]);
    const [tumRotalar, setTumRotalar] = useState([]);
    const [secilenRotaIndex, setSecilenRotaIndex] = useState(0);
    console.log("Durak sayısı:", duraklar.length);
    const startStop = await getEnYakinDurak(userLocation);
    const endStop = await getEnYakinDurak(destination);
    console.log("Başlangıç durağı:", startStop.data);
    console.log("Hedef durağı:", endStop.data);

    useEffect(() => {
        getDuraklar()
            .then(res => setDuraklar(res.data))
            .catch(err => console.error(err));
    }, []);

    const indirimOrani = { Genel: 0, Öğrenci: 0.5, Yaşlı: 1 };
    const odemeCarpani = { Nakit: 1, "Kredi Kartı": 1.1, Kentkart: 0.95 };

    const calculateRoutes = async () => {
        setMesaj('');
        setTumRotalar([]);
        setRotaKoordinatlari([]);

        if (!userLocation || !destination) {
            alert("Lütfen başlangıç ve hedef konumlarını seçin.");
            return;
        }

        const startStop = await getEnYakinDurak(userLocation);
        const endStop = await getEnYakinDurak(destination);

        const rotalar = [
            {
                tip: 'En Hızlı',
                adimlar: [
                    { from: 'Başlangıç', to: startStop.data.name, type: 'Taksi', sure: 4, mesafe: 2, ucret: 25, coords: [[userLocation.lat, userLocation.lng], [startStop.data.konum.latitude, startStop.data.konum.longitude]] },
                    { from: startStop.data.name, to: endStop.data.name, type: 'Otobüs', sure: 10, mesafe: 5, ucret: 8, coords: [[startStop.data.konum.latitude, startStop.data.konum.longitude], [endStop.data.konum.latitude, endStop.data.konum.longitude]] }
                ]
            },
            {
                tip: 'En Ucuz',
                adimlar: [
                    { from: startStop.data.name, to: endStop.data.name, type: 'Otobüs', sure: 15, mesafe: 6, ucret: 6, coords: [[startStop.data.konum.latitude, startStop.data.konum.longitude], [endStop.data.konum.latitude, endStop.data.konum.longitude]] }
                ]
            },
            {
                tip: 'Sadece Taksi',
                adimlar: [
                    { from: 'Başlangıç', to: 'Hedef', type: 'Taksi', sure: 12, mesafe: 5, ucret: 40, coords: [[userLocation.lat, userLocation.lng], [destination.lat, destination.lng]] }
                ]
            },
            {
                tip: 'Yürüyerek',
                adimlar: [
                    { from: 'Başlangıç', to: 'Hedef', type: 'Yürüyerek', sure: 40, mesafe: 4, ucret: 0, coords: [[userLocation.lat, userLocation.lng], [destination.lat, destination.lng]] }
                ]
            }
        ];

        // Hesaplama
        const oran = indirimOrani[yolcuTipi] || 0;
        const carpani = odemeCarpani[odemeTipi] || 1;

        const hesaplanmisRotalar = rotalar.map(rota => {
            let toplamSure = 0;
            let toplamMesafe = 0;
            let toplamUcret = 0;

            const adimlar = rota.adimlar.map(adim => {
                const indirimliUcret = adim.ucret * (1 - oran) * carpani;
                toplamSure += adim.sure;
                toplamMesafe += adim.mesafe;
                toplamUcret += indirimliUcret;

                return { ...adim, indirimliUcret: indirimliUcret.toFixed(2) };
            });

            return {
                tip: rota.tip,
                toplamSure,
                toplamMesafe,
                toplamUcret: toplamUcret.toFixed(2),
                adimlar
            };
        });

        // Kentkart kontrolü
        const secili = hesaplanmisRotalar[secilenRotaIndex];
        if (odemeTipi === 'Kentkart' && parseFloat(bakiye) < parseFloat(secili.toplamUcret)) {
            setMesaj(`❌ Bakiye yetersiz! Gerekli: ${secili.toplamUcret} TL`);
            return;
        }

        setTumRotalar(hesaplanmisRotalar);
        setSecilenRotaIndex(0);
        setRotaKoordinatlari(hesaplanmisRotalar[0].adimlar.flatMap(a => a.coords));
        setMesaj(`✅ ${hesaplanmisRotalar[0].tip} rota seçildi`);
    };

    const seciliRota = tumRotalar[secilenRotaIndex];

    return (
        <div style={{ padding: 20 }}>
            <h2>🚍 İzmit Ulaşım Sistemi</h2>

            <div style={{ display: 'flex', gap: 20 }}>
                <div style={{ flex: 1 }}>
                    <h4>📍 Başlangıç</h4>
                    <MapComponent duraklar={duraklar} onLocationSelect={setUserLocation} rota={rotaKoordinatlari} />
                </div>
                <div style={{ flex: 1 }}>
                    <h4>🎯 Hedef</h4>
                    <MapComponent duraklar={duraklar} onLocationSelect={setDestination} rota={rotaKoordinatlari} />
                </div>
            </div>

            <hr />

            <label>👤 Yolcu Tipi: </label>
            <select value={yolcuTipi} onChange={e => setYolcuTipi(e.target.value)}>
                <option>Genel</option>
                <option>Öğrenci</option>
                <option>Yaşlı</option>
            </select>

            <label style={{ marginLeft: 20 }}>💳 Ödeme Tipi: </label>
            <select value={odemeTipi} onChange={e => setOdemeTipi(e.target.value)}>
                <option>Nakit</option>
                <option>Kredi Kartı</option>
                <option>Kentkart</option>
            </select>

            {odemeTipi === 'Kentkart' && (
                <>
                    <label style={{ marginLeft: 20 }}>💰 Bakiye: </label>
                    <input type="number" value={bakiye} onChange={e => setBakiye(e.target.value)} />
                </>
            )}

            <button style={{ marginLeft: 20 }} onClick={calculateRoutes}>🚀 Rotayı Hesapla</button>

            {mesaj && <p>{mesaj}</p>}

            {seciliRota && (
                <>
                    <h3>📋 {seciliRota.tip} Rota Detayları</h3>
                    <ul>
                        {seciliRota.adimlar.map((a, i) => (
                            <li key={i}>
                                <strong>{a.type}:</strong> {a.from} → {a.to} | ⏱️ {a.sure} dk | 📏 {a.mesafe} km | 💸 {a.indirimliUcret} TL
                            </li>
                        ))}
                    </ul>
                    <strong>
                        Toplam: ⏱️ {seciliRota.toplamSure} dk | 📏 {seciliRota.toplamMesafe} km | 💸 {seciliRota.toplamUcret} TL
                    </strong>

                    <h4>🔄 Alternatif Rotalar:</h4>
                    {tumRotalar.map((rota, index) => (
                        <div key={index}>
                            <input
                                type="radio"
                                name="rotaSec"
                                checked={secilenRotaIndex === index}
                                onChange={() => {
                                    setSecilenRotaIndex(index);
                                    setRotaKoordinatlari(rota.adimlar.flatMap(a => a.coords));
                                    setMesaj(`✅ ${rota.tip} rota seçildi`);
                                }}
                            />
                            <label>
                                {rota.tip} | ⏱️ {rota.toplamSure} dk | 📏 {rota.toplamMesafe} km | 💸 {rota.toplamUcret} TL
                            </label>
                        </div>
                    ))}
                </>
            )}
        </div>
    );
}

export default App;
