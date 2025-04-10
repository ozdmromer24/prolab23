import axios from 'axios';

const API_URL = 'http://localhost:5000/api'; // backend API adresini kontrol et

export const getDuraklar = () => axios.get(`${API_URL}/durak`);

export const getEnYakinDurak = (konum) => axios.post(`${API_URL}/durak/enyakin`, konum);

export const rotaHesapla = (baslangic, hedef, yolcuTipi) =>
    axios.post(`${API_URL}/rota/hesapla`, { baslangic, hedef, yolcuTipi });
