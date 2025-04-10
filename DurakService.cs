using IzmitUlasimAPI.Models;
using Newtonsoft.Json;

namespace IzmitUlasimAPI.Services;

public class DurakService
{
    private List<Durak> _duraklar;

    public DurakService()
    {
        string json = File.ReadAllText("Data/veri.json");
        dynamic data = JsonConvert.DeserializeObject(json);

        _duraklar = new List<Durak>();

        foreach (var d in data.duraklar)
        {
            Durak durak = new Durak
            {
                Id = d.id,
                Name = d.name,
                Konum = new Konum { Latitude = d.lat, Longitude = d.lon },
                NextStops = new List<NextStop>()
            };

            foreach (var ns in d.nextStops)
            {
                durak.NextStops.Add(new NextStop
                {
                    StopId = ns.stopId,
                    Mesafe = ns.mesafe,
                    Sure = ns.sure,
                    Ucret = ns.ucret
                });
            }

            _duraklar.Add(durak);
        }
    }
    public RotaSonuc DijkstraRotaHesapla(Konum baslangic, Konum hedef, string yolcuTipi)
    {
        Durak baslangicDurak = EnYakinDuragiBul(baslangic);
        Durak hedefDurak = EnYakinDuragiBul(hedef);

        var rota = EnIyiRotayiBul(baslangicDurak, hedefDurak, "sure");

        double toplamSure = 0, toplamUcret = 0, toplamMesafe = 0;
        var durakDetaylari = new List<dynamic>();

        foreach (var durak in rota)
        {
            foreach (var next in durak.NextStops)
            {
                toplamSure += next.Sure;
                toplamMesafe += next.Mesafe;
                double indirimliUcret = next.Ucret * (yolcuTipi == "Öğrenci" ? 0.5 : yolcuTipi == "Yaşlı" ? 0 : 1);
                toplamUcret += indirimliUcret;

                durakDetaylari.Add(new
                {
                    from = durak.Name,
                    to = duraklar.First(d => d.Id == next.StopId).Name,
                    type = durak.Type,
                    sure = next.Sure,
                    ucret = indirimliUcret
                });
            }
        }

        return new RotaSonuc
        {
            anaRota = new
            {
                duraklar = durakDetaylari,
                toplamSure,
                toplamUcret,
                toplamMesafe
            },
            alternatifRotalar = new List<dynamic>
        {
            new { tip = "Sadece Taksi", sure = 8, mesafe = toplamMesafe, ucret = 50 },
            new { tip = "Sadece Otobüs", sure = toplamSure + 5, mesafe = toplamMesafe + 0.3, ucret = toplamUcret - 1 },
            new { tip = "En Az Aktarmalı", sure = toplamSure - 2, mesafe = toplamMesafe, ucret = toplamUcret + 0.75 }
        }
        };
    }

    public class RotaSonuc
    {
        public dynamic anaRota { get; set; }
        public List<dynamic> alternatifRotalar { get; set; }
    }

    public List<Durak> GetAll() => _duraklar;

    public Durak GetById(string id) => _duraklar.FirstOrDefault(d => d.Id == id);

    public Durak EnYakinDuragiBul(Konum konum)
        => _duraklar.OrderBy(d => d.Konum.MesafeHesapla(konum)).First();
}
