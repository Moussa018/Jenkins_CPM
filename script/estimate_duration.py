import argparse
import json
import os
import statistics
from collections import defaultdict

import requests

def get_last_build_numbers(base_url , job_name , count , auth):
    
    url = f"{base_url}/job/{job_name}/api/json"
    params = {"tree": "builds[number]"}
    
    response = requests.get(url , params= params , auth=auth)
    response.raise_for_status()
    
    data = response.json()    
    build_numbers  = [b["number"] for b in data["builds"]]
    
    return sorted(build_numbers, reverse=True)[:count]

def get_stage_duration_for_build(base_url , job_name , build_number, auth):
    url = f"{base_url}/job/{job_name}/{build_number}/wfapi/describe"
    
    response = requests.get(url, auth=auth)
    response.raise_for_status()
    
    data = response.json()
    durations = {}
    
    for stage in data.get("stages", []):
        name = stage["name"]
        duration_ms = stage.get("durationMillis", 0)
        durations[name] = duration_ms/1000.0
    return durations


def estimate_duration(base_url , job_name , build_count , auth):
    build_numbers = get_last_build_numbers(base_url=base_url, job_name=job_name,count=build_count, auth=auth)
    
    if not build_numbers:
        raise RuntimeError("Aucune build trouvé pour ce job")
    
    all_duration = defaultdict(list)
    # Ordre d'exécution des stages, repris de la build la plus récente exploitable.
    # C'est lui qui donne les dépendances : Jenkins exécute les stages en séquence.
    stage_order = []
    
    for build_number in build_numbers:
        try:
        
            stage_durations = get_stage_duration_for_build(base_url , job_name, build_number , auth)
        
        except requests.exceptions.HTTPError:
            print(f"Build {build_number} ignoré")
            continue
        
        if not stage_order:
            stage_order = list(stage_durations.keys())
        
        for stage_name , duration in stage_durations.items():
        
            all_duration[stage_name].append(duration)
    
    # Un stage absent de la build de référence est ajouté à la fin
    for stage_name in all_duration:
        if stage_name not in stage_order:
            stage_order.append(stage_name)
    
    estimated = {}
    
    for stage_name , duration_list in all_duration.items():
        
        estimated[stage_name] = statistics.mean(duration_list)
    
    return estimated , stage_order

def write_pipeline_json(estimated_durations , stage_order , output_path):
    stages = []
    previous = None
    
    for name in stage_order:
        stages.append({
            "name": name,
            "duration": round(estimated_durations[name] , 2),
            # Le pipeline observé est séquentiel : chaque stage dépend du précédent.
            # C'est cette chaîne que le solveur CPM analyse pour proposer une parallélisation.
            "dependencies": [previous] if previous else []
        })
        previous = name
    
    output = {"stages": stages}
    
    directory = os.path.dirname(output_path)
    if directory:
        os.makedirs(directory , exist_ok=True)
    
    with  open(output_path , "w" , encoding="utf-8") as f :
        json.dump(output , f , indent=2 , ensure_ascii=False)
    print(f"Résultat écrit dans : {output_path}")
    
def main():
    parser = argparse.ArgumentParser(
        description="Estime les durées de stages Jenkins à partir de l'historique des builds."  
    )
    parser.add_argument("--url", required=True, help="URL de base de Jenkins, ex: http://localhost:8080")
    parser.add_argument("--job", required=True, help="Nom du job/pipeline Jenkins")
    parser.add_argument("--builds", type=int, default=10, help="Nombre de builds récents à analyser (défaut : 10)")
    parser.add_argument("--user", default=None, help="Nom d'utilisateur Jenkins (si authentification requise)")
    parser.add_argument("--token", default=None, help="API token Jenkins (si authentification requise)")
    parser.add_argument("--output", default="example/pipeline.json", help="Chemin du fichier JSON de sortie")
    
    args = parser.parse_args()
    
    auth = (args.user , args.token) if args.user and args.token else None
    
    print(f"Analyse des {args.builds} derniers builds de '{args.job}'...")
    
    estimated , stage_order = estimate_duration(args.url , args.job , args.builds , auth)
    
    print("\nDurées estimées par stage :")
    for name in stage_order:
        print(f"  {name} : {estimated[name]} secondes")
    
    write_pipeline_json(estimated, stage_order, args.output)

if __name__ == "__main__" :
    main()
