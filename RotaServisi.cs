using UlasimSistemi.Models;

namespace UlasimSistemi.Services
{
	public class RotaServisi
	{
		private readonly List<Durak> _duraklar;
		private readonly TaxiInfo _taksi;

		public RotaServisi(List<Durak> duraklar, TaxiInfo taksi)
		{
			_duraklar = duraklar;
			_taksi = taksi;
		}

		public dynamic DijkstraRotaHesapla(Konum baslangic, Konum hedef, string yolcuTipi)
		{
			var baslangicDurak = _duraklar.OrderBy(d => d.Konum.MesafeHesapla(baslangic)).First();
			var hedefDurak = _duraklar.OrderBy(d => d.Konum.MesafeHesapla(hedef)).First();

			var rota = new List<dynamic>();
			double toplamSure = 0, toplamMesafe = 0, toplamUcret = 0;

			// Varsayım: Başlangıç → Taksi → İlk Durak → Toplu taşıma → Hedef
			rota.Add(new
			{
				from = "Başlangıç",
				to = baslangicDurak.Name,
				type = "Taksi",
				sure = 5,
				mesafe = baslangic.KonumFarkKm(baslangicDurak.Konum),
				ucret = _taksi.openingFee + (_taksi.costPerKm * baslangic.KonumFarkKm(baslangicDurak.Konum))
			});

			rota.Add(new
			{
				from = baslangicDurak.Name,
				to = hedefDurak.Name,
				type = "Otobüs",
				sure = 10,
				mesafe = baslangicDurak.Konum.MesafeHesapla(hedefDurak.Konum),
				ucret = 6.0
			});

			double indirim = yolcuTipi == "Öğrenci" ? 0.5 : yolcuTipi == "Yaşlı" ? 0 : 1;

			foreach (var r in rota)
			{
				double ucret = r.ucret * indirim;
				toplamUcret += ucret;
				toplamMesafe += r.mesafe;
				toplamSure += r.sure;
			}

			return new
			{
				rota,
				toplamUcret,
				toplamSure,
				toplamMesafe
			};
		}
	}
}
